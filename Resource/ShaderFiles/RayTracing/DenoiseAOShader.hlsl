﻿
//#pragma enable_d3d11_debug_symbols

#include "RaytracingShaderHeader.hlsli"

// グローバルルートシグネチャ
RaytracingAccelerationStructure gRtScene : register(t0);
ConstantBuffer<SceneCB> gSceneParam : register(b0);

// ヒットグループのローカルルートシグネチャ
StructuredBuffer<uint> indexBuffer : register(t0, space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1, space1);
Texture2D<float4> texture : register(t2, space1);
// サンプラー
SamplerState smp : register(s0, space1);

// RayGenerationシェーダーのローカルルートシグネチャ
RWTexture2D<float4> aoOutput : register(u0);
RWTexture2D<float4> lightingOutput : register(u1);
RWTexture2D<float4> colorOutput : register(u2);
RWTexture2D<float4> giOutput : register(u3);

// 当たった位置の情報を取得する関数
Vertex GetHitVertex(MyAttribute attrib, StructuredBuffer<Vertex> vertexBuffer, StructuredBuffer<uint> indexBuffer)
{
    Vertex v = (Vertex) 0;
    float3 barycentrics = CalcBarycentrics(attrib.barys);
    uint vertexId = PrimitiveIndex() * 3; // Triangle List のため.

    float weights[3] =
    {
        barycentrics.x, barycentrics.y, barycentrics.z
    };

    for (int i = 0; i < 3; ++i)
    {
        uint index = indexBuffer[vertexId + i];
        float w = weights[i];
        v.Position += vertexBuffer[index].Position * w;
        v.Normal += vertexBuffer[index].Normal * w;
        v.uv += vertexBuffer[index].uv * w;
    }
    v.Normal = normalize(v.Normal);

    return v;
}

float Scale(float fcos)
{
    
    //fcos = saturate(fcos);
    
    float x = 1.0 - fcos;
    static const float fScaleDepth = 0.25;
    float buff = fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.8 + x * 5.25))));
    if (abs(buff) < 0.01f)
    {
        return 0;
    }
    else
    {
        return buff;
    }
}

float3 IntersectionPos(float3 dir, float3 a, float radius)
{
    float b = dot(a, dir);
    float c = dot(a, a) - radius * radius;
    float d = max(b * b - c, 0.0);

    return a + dir * (-b + sqrt(d));
}

// 大気散乱
float3 AtmosphericScattering(Vertex vtx)
{
    
    // 地平線以下は真っ黒にする。
    if (vtx.Position.y < 0)
    {
        return float3(0.1f, 0.1f, 0.1f);
    }
    
    // レイリー散乱定数
    float kr = gSceneParam.AS.kr;
    // ミー散乱定数
    float km = gSceneParam.AS.km;
 
    // 大気中の線分をサンプリングする数。
    float fSamples = gSceneParam.AS.samples;
 
    // 謎の色 色的には薄めの茶色
    float3 three_primary_colors = float3(0.68f, 0.55f, 0.44f);
    // 光の波長？
    float3 v3InvWaveLength = 1.0f / pow(three_primary_colors, 4.0f);
 
    // 大気圏の一番上の高さ。
    float fOuterRadius = gSceneParam.AS.outerRadius;
    // 地球全体の地上の高さ。
    float fInnerRadius = gSceneParam.AS.innerRadius;
 
    // 太陽光の強さ？
    float fESun = gSceneParam.AS.eSun;
    // 太陽光の強さにレイリー散乱定数をかけてレイリー散乱の強さを求めている。
    float fKrESun = kr * fESun;
    // 太陽光の強さにミー散乱定数をかけてレイリー散乱の強さを求めている。
    float fKmESun = km * fESun;

    // レイリー散乱定数に円周率をかけているのだが、限りなく0に近い値。
    float fKr4PI = kr * 4.0f * PI;
    // ミー散乱定数に円周率をかけているのだが、ミー散乱定数は0なのでこれの値は0。
    float fKm4PI = km * 4.0f * PI;
 
    // 地球全体での大気の割合。
    float fScale = 1.0f / (fOuterRadius - fInnerRadius);
    // 平均大気密度を求める高さ。
    float fScaleDepth = gSceneParam.AS.aveHeight;
    // 地球全体での大気の割合を平均大気密度で割った値。
    float fScaleOverScaleDepth = fScale / fScaleDepth;
 
    // 散乱定数を求める際に使用する値。
    float g = gSceneParam.AS.g;
    // 散乱定数を求める際に使用する値を二乗したもの。なぜ。
    float g2 = g * g;
    
    // 当たった天球のワールド座標
    float3 worldPos = normalize(mul(float4(vtx.Position, 1), ObjectToWorld4x3())) * fOuterRadius;
    //worldPos = IntersectionPos(normalize(worldPos), float3(0.0, fInnerRadius, 0.0), fOuterRadius);
    
    // カメラ座標 元計算式だと中心固定になってしまっていそう。
    float3 v3CameraPos = float3(0.0, 1.0f, 0.0f);
    //float3 v3CameraPos = WorldRayOrigin();
    
    // ディレクショナルライトの場所を求める。
    float3 dirLightPos = -gSceneParam.dirLight.lightDir * 1000000.0f;
    
    // ディレクショナルライトの方向を求める。
    float3 v3LightDir = normalize(worldPos - dirLightPos);
    //float3 v3LightDir = gSceneParam.dirLight.lightDir;
 
    // 天球上頂点からカメラまでのベクトル(光が大気圏に突入した点からカメラまでの光のベクトル)
    float3 v3Ray = worldPos - v3CameraPos;
    
    // 大気に突入してからの点とカメラまでの距離。
    float fFar = length(v3Ray);
    
    // 正規化された拡散光が来た方向。
    v3Ray /= fFar;
    
    //return v3Ray;
 
    // サンプリングする始点座標 資料だとAの頂点
    float3 v3Start = v3CameraPos;
    // サンプルではカメラの位置が(0,Radius,0)なのでカメラの高さ。どの位置に移動しても地球視点で見れば原点(地球の中心)からの高さ。
    float fCameraHeight = length(v3CameraPos);
    // 地上からの法線(?)と拡散光がやってきた角度の内積によって求められた角度をカメラの高さで割る。
    float fStartAngle = dot(v3Ray, normalize(v3Start)) / fCameraHeight;
    // 開始地点の高さに平均大気密度をかけた値の指数を求める？
    float fStartDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
    // 開始地点のなにかの角度のオフセット。
    float fStartOffset = fStartDepth * Scale(fStartAngle);
    // サンプルポイント間の長さ。
    float fSampleLength = fFar / fSamples;
    // サンプルポイント間の長さに地球の大気の割合をかける。何故。
    float fScaledLength = fSampleLength * fScale;
    // 拡散光が来た方向にサンプルの長さをかけることでサンプルポイント間のレイをベクトルを求める。
    float3 v3SampleRay = v3Ray * fSampleLength;
    // 最初のサンプルポイントを求める。0.5をかけてるのは謎。
    float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;
 
    // 色情報
    float3 v3FrontColor = 0.0f;
    for (int n = 0; n < int(fSamples); n++)
    {
        // サンプルポイントの高さ。どちらにせよ原点は地球の中心なので、この値が現在位置の高さになる。
        float fHeight = length(v3SamplePoint);
        // 地上からサンプルポイントの高さの差に平均大気密度をかけたもの。
        float fDepth = (fScaleOverScaleDepth * (fInnerRadius - fHeight));
        // 地上から見たサンプルポイントの法線とディレクショナルライトの方向の角度を求めて、サンプルポイントの高さで割る。
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        // 地上から見たサンプルポイントの法線と散乱光が飛んできている方区の角度を求めて、サンプルポイントの高さで割る。
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        // 散乱光？
        float fScatter = (fStartOffset + fDepth * (Scale(fLightAngle) - Scale(fCameraAngle)));
        
        //float divBuff = 1.0f;
        //float minus = 1;
        //float divTarget = fScatter;
        //if (fScatter <= 3.9f)
        //{
        //    return float3(1, 0, 0);
        //}
        //else
        //{
        //    return float3(0, 0, 1);
        //}
        //return float3(divTarget / divBuff * minus, divTarget / divBuff * minus, divTarget / divBuff * minus);
        
        // 色ごとの減衰率？
        //float3 v3Attenuate = exp(fScatter * (v3InvWaveLength * fKr4PI + fKm4PI));
        float3 v3Attenuate = saturate(fScatter) * v3InvWaveLength;
        // サンプルポイントの位置を考慮して散乱した色を求める。
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength) * 0.00001f;
        // サンプルポイントを移動させる。
        v3SamplePoint += v3SampleRay;
        
        //return v3FrontColor * 0.00001f;
        
    }
 
    // レイリー散乱に使用する色情報
    float3 c0 = v3FrontColor * (v3InvWaveLength * fKrESun);
    // ミー散乱に使用する色情報
    float3 c1 = v3FrontColor * fKmESun;
    // カメラ座標から天球の座標へのベクトル。
    float3 v3Direction = v3CameraPos - worldPos;
 
    float fcos = dot(v3LightDir, v3Direction) / length(v3Direction);
    float fcos2 = fcos * fcos;
 
    // レイリー散乱の明るさ。
    float rayleighPhase = 0.75f * (1.0f + fcos2);
    // ミー散乱の明るさ。
    float miePhase = 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fcos2) / pow(1.0f + g2 - 2.0f * g * fcos, 1.5f);
 
    // 最終結果の色
    float3 col = 1.0f;
    col.rgb = rayleighPhase * c0 + miePhase * c1;
    return col;
    
}

// ソフトシャドウ射出関数
float SoftShadow(Vertex vtx, float lightSize, float length)
{
    float3 worldPosition = mul(float4(vtx.Position, 1), ObjectToWorld4x3());
    
    // 光源への中心ベクトル
    float3 pointLightPosition = gSceneParam.pointLight.lightPos;
    float3 lightDir = normalize(pointLightPosition - worldPosition);
    
    // ライトベクトルと垂直なベクトルを求める。
    float3 perpL = cross(lightDir, float3(0, 1, 0));
    if (all(perpL == 0.0f))
    {
        perpL.x = 1.0f;
    }
    
    // 光源の端を求める。
    float3 toLightEdge = (pointLightPosition + perpL * lightSize) - worldPosition;
    toLightEdge = normalize(toLightEdge);
    
    // 角度を求める。
    float coneAngle = acos(dot(lightDir, toLightEdge)) * 2.0f;
    
    // 乱数の種を求める。
    uint2 pixldx = DispatchRaysIndex().xy;
    uint2 numPix = DispatchRaysDimensions().xy;
    int randSeed = initRand(DispatchRaysIndex().x + (vtx.Position.x / 1000.0f) + DispatchRaysIndex().y * numPix.x, 100);
    
    float3 shadowRayDir = GetConeSample(randSeed, lightDir, coneAngle);
    return ShootShadowRay(worldPosition, shadowRayDir, length, gRtScene);
    
}

// ソフトシャドウ射出関数
float ShootDirShadow(Vertex vtx, float length)
{
    float3 worldPosition = mul(float4(vtx.Position, 1), ObjectToWorld4x3());
    
    // ライトベクトルと垂直なベクトルを求める。
    float3 perpL = cross(-gSceneParam.dirLight.lightDir, float3(0, 1, 0));
    if (all(perpL == 0.0f))
    {
        perpL.x = 1.0f;
    }
    
    // 光源の端を求める。
    float3 toLightEdge = ((vtx.Position + -gSceneParam.dirLight.lightDir * 1000.0f) + perpL * 10) - worldPosition;
    toLightEdge = normalize(toLightEdge);
    
    // 角度を求める。
    float coneAngle = acos(dot(-gSceneParam.dirLight.lightDir, toLightEdge)) * 2.0f;
    
    // 乱数の種を求める。
    uint2 pixldx = DispatchRaysIndex().xy;
    uint2 numPix = DispatchRaysDimensions().xy;
    int randSeed = initRand(DispatchRaysIndex().x + (vtx.Position.x / 1000.0f) + DispatchRaysIndex().y * numPix.x, 100);
    
    float3 shadowRayDir = GetConeSample(randSeed, -gSceneParam.dirLight.lightDir, coneAngle);
    return ShootShadowRayNoAH(worldPosition, shadowRayDir, length, gRtScene);
    
}

// GI
float3 ShootGIRay(Vertex vtx, float length)
{
    float3 worldPos = mul(float4(vtx.Position, 1), ObjectToWorld4x3());
    float3 worldNormal = mul(vtx.Normal, (float3x3) ObjectToWorld4x3());
    float3 worldRayDir = WorldRayDirection();
    float3 reflectDir = reflect(worldRayDir, worldNormal);
    
    // レイのフラグを設定。
    RAY_FLAG flag = RAY_FLAG_NONE;
    //flag |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    //flag |= RAY_FLAG_FORCE_OPAQUE; // AnyHitShaderを無視。
    
    // レイのマスク
    uint rayMask = 0xFF;

    // レイのパラメーターを設定。
    RayDesc rayDesc;
    rayDesc.Origin = worldPos;
    rayDesc.Direction = reflectDir;
    rayDesc.TMin = 0.01;
    rayDesc.TMax = length;

    // ペイロードを初期化。
    DenoisePayload reflectPayload;
    reflectPayload.color = float3(0, 0, 0);
    reflectPayload.aoLuminance = float3(0, 0, 0);
    reflectPayload.lightLuminance = float3(0, 0, 0);
    reflectPayload.giColor = float3(0, 0, 0);
    reflectPayload.recursive = 100000;
    
    // レイを発射。
    TraceRay(
        gRtScene,
        flag,
        rayMask,
        0, // ray index
        1, // MultiplierForGeometryContrib
        0, // miss index
        rayDesc,
        reflectPayload);
    return reflectPayload.giColor;
}

// RayGenerationシェーダー
[shader("raygeneration")]
void mainRayGen()
{

    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);

    float2 d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
    float aspect = dims.x / dims.y;

    matrix mtxViewInv = gSceneParam.mtxViewInv;
    matrix mtxProjInv = gSceneParam.mtxProjInv;

    // レイの設定
    RayDesc rayDesc;
    rayDesc.Origin = mul(mtxViewInv, float4(0, 0, 0, 1)).xyz;

    float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
    float3 dir = mul(mtxViewInv, float4(target.xyz, 0)).xyz;

    rayDesc.Direction = normalize(dir);
    rayDesc.TMin = 0;
    rayDesc.TMax = 100000;

    // ペイロードの設定
    DenoisePayload payload;
    payload.color = float3(0, 0, 0);
    payload.aoLuminance = float3(0, 0, 0);
    payload.lightLuminance = float3(0, 0, 0);
    payload.giColor = float3(0, 0, 0);
    payload.recursive = 0;

    // TransRayに必要な設定を作成
    uint rayMask = 0xFF;
    
    RAY_FLAG flag = RAY_FLAG_NONE;
    flag |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    //flag |= RAY_FLAG_FORCE_OPAQUE; // AnyHitShaderを無視。

    // レイを発射
    TraceRay(
    gRtScene, // TLAS
    flag, // 衝突判定制御をするフラグ
    rayMask, // 衝突判定対象のマスク値
    0, // ray index
    1, // MultiplierForGeometryContrib
    0, // miss index
    rayDesc,
    payload);
    
    
    // レイを発射した結果の色を取得
    float3 col = payload.color;

    // 結果格納
    lightingOutput[launchIndex.xy] = float4(payload.aoLuminance, 1);
    aoOutput[launchIndex.xy] = float4(payload.lightLuminance, 1);
    colorOutput[launchIndex.xy] = float4(payload.color, 1);
    giOutput[launchIndex.xy] = float4(payload.giColor, 1);

}

// missシェーダー レイがヒットしなかった時に呼ばれるシェーダー
[shader("miss")]
void mainMS(inout DenoisePayload payload)
{

    // 単色を返すようにする。
    payload.color = float3(0, 0, 0);
    //payload.color = float3(0xFF / 255.0f, 0xFF / 255.0f, 0xE5 / 255.0f);
    //payload.color = float3(0x32 / 255.0f, 0x90 / 255.0f, 0xD0 / 255.0f);

}

// シャドウ用missシェーダー
[shader("miss")]
void shadowMS(inout ShadowPayload payload)
{
    // 何にも当たっていないということなので、影は生成しない。
    payload.isShadow = true;
}

// closesthitシェーダー レイがヒットした時に呼ばれるシェーダー
[shader("closesthit")]
void mainCHS(inout DenoisePayload payload, MyAttribute attrib)
{
    
    Vertex vtx = GetHitVertex(attrib, vertexBuffer, indexBuffer);
    uint instanceID = InstanceID();

    // テクスチャの色を保存。
    float3 texColor = texture.SampleLevel(smp, vtx.uv, 0.0f);
    
    // 反射回数が100000回だったらGI用のレイなのでテクスチャの色を返す。
    if (payload.recursive == 100000)
    {
        // レイの長さ
        float rayLength = length(WorldRayOrigin() - vtx.Position);
        
        // レイの長さの最大値
        const float MAX_RAY = 500.0f;
        
        // 割合を求める。
        float rate = rayLength / MAX_RAY;
        rate = 1.0f - saturate(rate);
        
        payload.giColor = texColor * rate;
        return;
    }
    
    // Instance数が1だったらテクスチャの色をそのまま返す。
    if (instanceID == 1)
    {
        payload.lightLuminance = float3(1, 1, 1);
        //payload.color = texColor;
        payload.color = AtmosphericScattering(vtx);
        payload.aoLuminance = float3(1, 1, 1);
        payload.giColor = float3(1, 1, 1);
        return;
    }
    
    // ポリゴンの描画するフラグが立っていたら。
    if (gSceneParam.isMeshScene)
    {
        payload.lightLuminance = CalcBarycentrics(attrib.barys);
        return;
    }

    // 法線マップが存在していたら法線マップから法線情報を抽出。
    //vtx.Normal = normalMap.SampleLevel(smp, vtx.uv, 0.0f);
    
    // 法線を描画するフラグが立っていたら。
    if (gSceneParam.isNormalScene)
    {
        payload.lightLuminance = vtx.Normal;
        return;
    }
    
    // 呼び出し回数が制限を超えないようにする。
    ++payload.recursive;
    if (1 < payload.recursive)
    {
        return;
    }

    // 通常ライティング
    uint2 pixldx = DispatchRaysIndex().xy;
    uint2 numPix = DispatchRaysDimensions().xy;
    
    // ライティングの結果の明るさ
    float pointLightVisibility = 0;
    float dirLightVisibility = 0;
    float aoLightVisibility = 0;
    
    // ポイントライトのライティング結果の色
    float3 pointLightColor = float3(0, 0, 0);
    
    // 並行光源のライティング結果の色
    float3 dirLightColor = float3(0, 0, 0);
    
    // ライトまでの距離
    float lightLength = length(gSceneParam.pointLight.lightPos - vtx.Position);
    
    // 点光源へシャドウレイを飛ばす。
    if (lightLength < gSceneParam.pointLight.lightPower && gSceneParam.pointLight.isActive)
    {
        
        pointLightVisibility = SoftShadow(vtx, gSceneParam.pointLight.lightSize, length(gSceneParam.pointLight.lightPos - vtx.Position));
        
        // 影だったら
        if (0 <= pointLightVisibility)
        {
            
            // 明るさを減衰させる。
            float rate = lightLength / gSceneParam.pointLight.lightPower;
            rate = pow(rate, 5);
            rate = 1.0f - rate;
                
            // ランバートの反射率と明るさをかける。
            pointLightVisibility *= rate;
                
            pointLightColor += gSceneParam.pointLight.lightColor * pointLightVisibility;
                
            
        }
        
    }
    
    // 並行光源にシャドウレイを飛ばす。
    if (gSceneParam.dirLight.isActive)
    {
        
        dirLightVisibility = ShootDirShadow(vtx, 3000.0f);
        
        dirLightColor = gSceneParam.dirLight.lightColor * dirLightVisibility;
        
    }
    
        
    // 飛ばすレイの回数
    for (int index = 0; index < 1; ++index)
    {
        // アンビエントオクリュージョンを行わないフラグが立っていたら処理を飛ばす。
        if (gSceneParam.isNoAO)
        {
            break;
        }
            
       
        int seed = initRand(DispatchRaysIndex().x + (vtx.Position.x / 1000.0f) + index + DispatchRaysIndex().y * numPix.x, 100);
        float3 sampleDir = GetUniformHemisphereSample(seed, vtx.Normal);
        
        // シャドウレイを飛ばす。
        float aoLightVisibilityBuff = ShootAOShadowRay(vtx.Position, sampleDir, 100, gRtScene);
        
        // 隠蔽度合い += サンプリングした値 * コサイン項 / 確率密度関数
        float NoL = saturate(dot(vtx.Normal, sampleDir));
        float pdf = 1.0 / (2.0 * PI);
        aoLightVisibility += aoLightVisibilityBuff * NoL / pdf;
        
            
    }
    // 平均を取る。
    aoLightVisibility = (1.0f / PI) * (1.0f / float(1)) * aoLightVisibility;
        
    
    
    // 光源へのライティングを加算する。
    float lightVisibility = 0;
    lightVisibility += pointLightVisibility + dirLightVisibility;
    float aoVisibility = aoLightVisibility;
    
    // 最終結果の色を保存。
    payload.color.xyz = texColor;
    payload.lightLuminance = lightVisibility + (pointLightColor + dirLightColor) / PI;
    payload.aoLuminance = aoVisibility;
    
    // GIの色を取得する。
    if (instanceID == 10 && !gSceneParam.isNoGI)
    {
        payload.giColor = ShootGIRay(vtx, 500) * 1.0f;
    }
    else
    {
        payload.giColor = float3(0, 0, 0);
    }
    
    // GIのみを描画するフラグが立っていたらGI以外の色を無効化する。
    if (gSceneParam.isGIOnlyScene)
    {
        
        payload.lightLuminance = float3(1, 1, 1);
        payload.color = float3(0, 0, 0);
        payload.aoLuminance = float3(0, 0, 0);
        
    }
    
    // ライトに当たった面だけ表示するフラグが立っていたら。
    if (gSceneParam.isLightHitScene)
    {
        
        // 光にあたっていたら。
        if (0.0f < pointLightVisibility + dirLightVisibility)
        {
            payload.color = float3(1, 1, 1);
        }
        else
        {
            payload.color = float3(0, 0, 0);
        }
        
        return;
        
    }

}

// closesthitシェーダー シャドウ用
[shader("closesthit")]
void shadowCHS(inout ShadowPayload payload, MyAttribute attrib)
{
}

// アルファ抜きAnyHitShader
[shader("anyhit")]
void mainAnyHit(inout DenoisePayload payload, MyAttribute attrib)
{
    Vertex vtx = GetHitVertex(attrib, vertexBuffer, indexBuffer);
    float4 diffuse = texture.SampleLevel(smp, vtx.uv, 0);
    if (diffuse.w < 0.5f)
    {
        IgnoreHit();

    }
    
    int instanceID = InstanceID();
    
    // インスタンスIDが1(ライト)なら当たり判定を棄却する。
    if (instanceID == 1)
    {
        IgnoreHit();

    }
    
}