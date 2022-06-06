
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
    rayDesc.TMax = 10000;

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
        payload.color = texColor;
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
