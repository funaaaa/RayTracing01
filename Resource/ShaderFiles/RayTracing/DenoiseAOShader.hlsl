
//#pragma enable_d3d11_debug_symbols

#include "RaytracingShaderHeader.hlsli"

// TLAS
RaytracingAccelerationStructure gRtScene : register(t0);
ConstantBuffer<ConstBufferData> gSceneParam : register(b0);

// 各リソース等
StructuredBuffer<uint> indexBuffer : register(t0, space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1, space1);
StructuredBuffer<Material> material : register(t2, space1);
Texture2D<float4> texture : register(t3, space1);
Texture2D<float4> mapTexture : register(t4, space1);
RWTexture2D<float4> tireMaskTexture : register(u0, space1);
// サンプラー
SamplerState smp : register(s0, space1);

// 出力先UAV
RWTexture2D<float4> aoOutput : register(u0);
RWTexture2D<float4> lightingOutput : register(u1);
RWTexture2D<float4> colorOutput : register(u2);
RWTexture2D<float4> giOutput : register(u3);
RWTexture2D<float4> denoiseMaskoutput : register(u4);

// 大気散乱
float3 AtmosphericScattering(float3 pos, inout float3 mieColor)
{

    // レイリー散乱定数
    float kr = 0.0025f;
    // ミー散乱定数
    float km = 0.005f;

    // 大気中の線分をサンプリングする数。
    float fSamples = 2.0f;

    // 謎の色 色的には薄めの茶色
    float3 three_primary_colors = float3(0.68f, 0.55f, 0.44f);
    // 光の波長？
    float3 v3InvWaveLength = 1.0f / pow(three_primary_colors, 4.0f);

    // 大気圏の一番上の高さ。
    float fOuterRadius = 10250.0f;
    // 地球全体の地上の高さ。
    float fInnerRadius = 10000.0f;

    // 太陽光の強さ？
    float fESun = 20.0f;
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
    float fScaleDepth = 0.35f;
    // 地球全体での大気の割合を平均大気密度で割った値。
    float fScaleOverScaleDepth = fScale / fScaleDepth;

    // 散乱定数を求める際に使用する値。
    float g = -0.999f;
    // 散乱定数を求める際に使用する値を二乗したもの。なぜ。
    float g2 = g * g;

    // 当たった天球のワールド座標
    float3 worldPos = normalize(pos) * fOuterRadius;
    worldPos = IntersectionPos(normalize(worldPos), float3(0.0, fInnerRadius, 0.0), fOuterRadius);

    // カメラ座標 元計算式だと中心固定になってしまっていそう。
    float3 v3CameraPos = float3(0.0, fInnerRadius + 1.0f, 0.0f);

    // ディレクショナルライトの場所を求める。
    float3 dirLightPos = -gSceneParam.light.dirLight.lightDir * 1000000.0f;

    // ディレクショナルライトへの方向を求める。
    float3 v3LightDir = normalize(dirLightPos - worldPos);

    // 天球上頂点からカメラまでのベクトル(光が大気圏に突入した点からカメラまでの光のベクトル)
    float3 v3Ray = worldPos - v3CameraPos;

    // 大気に突入してからの点とカメラまでの距離。
    float fFar = length(v3Ray);

    // 正規化された拡散光が来た方向。
    v3Ray /= fFar;

    // サンプリングする始点座標 資料だとAの頂点
    float3 v3Start = v3CameraPos;
    // サンプルではカメラの位置が(0,Radius,0)なのでカメラの高さ。どの位置に移動しても地球視点で見れば原点(地球の中心)からの高さ。
    float fCameraHeight = length(v3CameraPos);
    // 地上からの法線(?)と拡散光がやってきた角度の内積によって求められた角度をカメラの高さで割る。
    float fStartAngle = dot(v3Ray, v3Start) / fCameraHeight;
    // 開始地点の高さに平均大気密度をかけた値の指数を求める？
    float fStartDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fCameraHeight));
    // 開始地点のなにかの角度のオフセット。
    float fStartOffset = fStartDepth * Scale(fStartAngle);

    // サンプルポイント間の長さ。
    float fSampleLength = fFar / fSamples;
    // サンプルポイント間の長さに地球の大気の割合をかける。
    float fScaledLength = fSampleLength * fScale;
    // 拡散光が来た方向にサンプルの長さをかけることでサンプルポイント間のレイをベクトルを求める。
    float3 v3SampleRay = v3Ray * fSampleLength;
    // 最初のサンプルポイントを求める。0.5をかけてるのは少し動かすため？
    float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;

    // 色情報
    float3 v3FrontColor = 0.0f;
    for (int n = 0; n < int(fSamples); ++n)
    {
        // サンプルポイントの高さ。どちらにせよ原点は地球の中心なので、この値が現在位置の高さになる。
        float fHeight = length(v3SamplePoint);
        // 地上からサンプルポイントの高さの差に平均大気密度をかけたもの。  高度に応じて大気密度が指数的に小さくなっていくのを表現している？
        float fDepth = exp(fScaleOverScaleDepth * (fInnerRadius - fHeight));
        // 地上から見たサンプルポイントの法線とディレクショナルライトの方向の角度を求めて、サンプルポイントの高さで割る。
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight; // こいつの値が-1になる→Scale内の計算でexpの引数が43になり、とてつもなくでかい値が入る。 → -にならないようにする？
        // 地上から見たサンプルポイントの法線と散乱光が飛んできている方区の角度を求めて、サンプルポイントの高さで割る。
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        // 散乱光？
        float fScatter = (fStartOffset + fDepth * (Scale(fLightAngle * 1) - Scale(fCameraAngle * 1)));

        // 色ごとの減衰率？
        float3 v3Attenuate = exp(-fScatter * (v3InvWaveLength * fKr4PI + fKm4PI));
        // サンプルポイントの位置を考慮して散乱した色を求める。
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        // サンプルポイントを移動させる。
        v3SamplePoint += v3SampleRay;

    }

    // レイリー散乱に使用する色情報
    float3 c0 = v3FrontColor * (v3InvWaveLength * fKrESun);
    // ミー散乱に使用する色情報
    float3 c1 = v3FrontColor * fKmESun;
    // カメラ座標から天球の座標へのベクトル。
    float3 v3Direction = v3CameraPos - worldPos;

    //float fcos = dot(v3LightDir, v3Direction) / length(v3Direction);
    float fcos = dot(v3LightDir, v3Direction) / length(v3Direction);
    float fcos2 = fcos * fcos;

    // レイリー散乱の明るさ。
    float rayleighPhase = 0.75f * (1.0f + fcos2);
    // ミー散乱の明るさ。
    float miePhase = 1.5f * ((1.0f - g2) / (2.0f + g2)) * (1.0f + fcos2) / pow(1.0f + g2 - 2.0f * g * fcos, 1.5f);

    // ミー散乱の色を保存。
    mieColor = c0 * rayleighPhase;

    // 交点までのベクトルと太陽までのベクトルが近かったら白色に描画する。
    if (0.999f < dot(normalize(dirLightPos - v3CameraPos), normalize(worldPos - v3CameraPos)))
    {
        return float3(1, 1, 1);
    }

    // 最終結果の色
    float3 col = 1.0f;
    col.rgb = rayleighPhase * c0 + miePhase * c1;
    return col;

}

// ソフトシャドウ射出関数
float SoftShadow(Vertex vtx, float lightSize, float length, int lightIndex)
{
    float3 worldPosition = mul(float4(vtx.Position, 1), ObjectToWorld4x3());

    // 光源への中心ベクトル
    float3 pointLightPosition = gSceneParam.light.pointLight[lightIndex].lightPos;
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
    int randSeed = InitRand(DispatchRaysIndex().x + (vtx.Position.x / 1000.0f) + DispatchRaysIndex().y * numPix.x, 100);
    
    float3 shadowRayDir = GetConeSample(randSeed, lightDir, coneAngle);
    return ShootShadowRay(worldPosition, shadowRayDir, length, gRtScene);
    
}

// 太陽光の影チェック用レイの準備関数 戻り値は太陽光の色
bool ShootDirShadow(Vertex vtx, float length)
{
    float3 worldPosition = mul(float4(vtx.Position, 1), ObjectToWorld4x3());

    // ライトベクトルと垂直なベクトルを求める。
    float3 perpL = cross(-gSceneParam.light.dirLight.lightDir, float3(0, 1, 0));
    if (all(perpL == 0.0f))
    {
        perpL.x = 1.0f;
    }

    // 並行光源の座標を仮で求める。
    float3 dirLightPos = -gSceneParam.light.dirLight.lightDir * 300000.0f;

    // 並行光源までのベクトル。
    float3 dirLightVec = dirLightPos - worldPosition;
    dirLightVec = normalize(dirLightVec);

    float3 shadowRayDir = dirLightVec;
    
    return ShootShadowRayNoAH(worldPosition + normalize(mul(vtx.Normal, (float3x3) ObjectToWorld4x3())) * 2.0f, shadowRayDir, length, gRtScene);
    
}

// GI
void ShootGIRay(Vertex vtx, float length, inout Payload PayloadData, float3 WorldNormal)
{
    float3 worldPos = mul(float4(vtx.Position, 1), ObjectToWorld4x3());
    float3 worldRayDir = WorldRayDirection();
    float3 reflectDir = reflect(worldRayDir, WorldNormal);
    
    // レイのフラグを設定。
    RAY_FLAG flag = RAY_FLAG_NONE;
    flag |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    //flag |= RAY_FLAG_FORCE_OPAQUE; // AnyHitShaderを無視。

    // レイのマスク
    uint rayMask = 0xFF;

    // レイのパラメーターを設定。
    RayDesc rayDesc;
    rayDesc.Origin = worldPos;
    rayDesc.Direction = reflectDir;
    rayDesc.TMin = 0.0;
    rayDesc.TMax = length;

    // ペイロードを初期化。
    int rayID = PayloadData.rayID_;
    PayloadData.rayID_ = CHS_IDENTIFICATION_RAYID_GI;
    
    // レイを発射。
    TraceRay(
        gRtScene,
        flag,
        rayMask,
        0, // ray index
        1, // MultiplierForGeometryContrib
        0, // miss index
        rayDesc,
        PayloadData);
    
    PayloadData.rayID_ = rayID;
    
}

// RayGenerationシェーダー
[shader("raygeneration")]
void mainRayGen()
{

    uint2 launchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);

    float2 d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
    float aspect = dims.x / dims.y;

    matrix mtxViewInv = gSceneParam.camera.mtxViewInv;
    matrix mtxProjInv = gSceneParam.camera.mtxProjInv;

    // レイの設定
    RayDesc rayDesc;
    rayDesc.Origin = mul(mtxViewInv, float4(0, 0, 0, 1)).xyz;

    float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
    float3 dir = mul(mtxViewInv, float4(target.xyz, 0)).xyz;

    rayDesc.Direction = normalize(dir);
    rayDesc.TMin = 0;
    rayDesc.TMax = 300000;

    // ペイロードの設定
    Payload payloadData;
    payloadData.impactAmount_ = 1.0f;
    payloadData.rayID_ = CHS_IDENTIFICATION_RAYID_DEF;
    payloadData.recursive_ = 0;
    payloadData.ao_ = 0;
    payloadData.color_ = float3(0, 0, 0);
    payloadData.denoiseMask_ = float3(0, 0, 0);
    payloadData.gi_ = float3(0, 0, 0);
    payloadData.light_ = float3(0, 0, 0);
    payloadData.isCullingAlpha_ = false;
    payloadData.alphaCounter_ = 0;
    payloadData.roughnessOffset_ = 1.0f;
    payloadData.pad_ = 0.0f;

    // TransRayに必要な設定を作成
    uint rayMask = 0xFF;
    
    RAY_FLAG flag = RAY_FLAG_NONE;
    flag |= RAY_FLAG_CULL_BACK_FACING_TRIANGLES;
    //flag |= RAY_FLAG_FORCE_NON_OPAQUE; // AnyHitShaderを実行。

    // レイを発射
    TraceRay(
    gRtScene, // TLAS
    flag, // 衝突判定制御をするフラグ
    rayMask, // 衝突判定対象のマスク値
    0, // ray index
    1, // MultiplierForGeometryContrib
    0, // miss index
    rayDesc,
    payloadData);

    // 結果書き込み用UAVを一旦初期化。
    lightingOutput[launchIndex.xy] = float4(1, 1, 1, 1);
    aoOutput[launchIndex.xy] = float4(0, 0, 0, 1);
    colorOutput[launchIndex.xy] = float4(1, 1, 1, 1);
    giOutput[launchIndex.xy] = float4(1, 1, 1, 1);
    denoiseMaskoutput[launchIndex.xy] = float4(1, 1, 1, 1);

    // Linear -> sRGB
    payloadData.light_ = 1.055f * pow(payloadData.light_, 1.0f / 2.4f) - 0.055f;
    payloadData.ao_ = 1.055f * pow(payloadData.ao_, 1.0f / 2.4f) - 0.055f;
    //payloadData.color_ = pow(payloadData.color_, 1.0f / 2.2f);
    //payloadData.gi_ = pow(payloadData.gi_, 1.0f / 2.2f);

    // 結果格納
    lightingOutput[launchIndex.xy] = float4(saturate(payloadData.light_), 1);
    aoOutput[launchIndex.xy] = float4(saturate(payloadData.ao_), saturate(payloadData.ao_), saturate(payloadData.ao_), 1);
    colorOutput[launchIndex.xy] = float4(saturate(payloadData.color_), 1);
    giOutput[launchIndex.xy] = float4(saturate(payloadData.gi_), 1);
    denoiseMaskoutput[launchIndex.xy] = float4(payloadData.denoiseMask_, 1);

}

// missシェーダー レイがヒットしなかった時に呼ばれるシェーダー
[shader("miss")]
void mainMS(inout Payload PayloadData)
{
    
    if (PayloadData.rayID_ == CHS_IDENTIFICATION_RAYID_GI || PayloadData.rayID_ == CHS_IDENTIFICATION_RAYID_SHADOW)
        return;
    
    // ペイロード受け取り用変数。
    Payload payloadBuff = PayloadData;
    
    // 影響度をかけつつ色を保存。
    float3 mieColor = float3(1, 1, 1);
    payloadBuff.light_ += float3(1, 1, 1) * payloadBuff.impactAmount_;
    payloadBuff.color_ += AtmosphericScattering(WorldRayOrigin() + WorldRayDirection() * RayTCurrent(), mieColor) * payloadBuff.impactAmount_ * payloadBuff.impactAmount_;
    payloadBuff.ao_ += 1.0f * payloadBuff.impactAmount_;
    payloadBuff.gi_ += float3(0, 0, 0) * payloadBuff.impactAmount_;
        
    // マスクの色を白くする。(ライトリーク対策で他のマスクの色とかぶらないようにするため。)
    payloadBuff.denoiseMask_ = float3(1, 1, 1);
        
    // サンプリングした点の輝度を取得する。
    float t = dot(payloadBuff.color_.xyz, float3(0.2125f, 0.7154f, 0.0721f));
        
    // サンプリングした点が地上より下じゃなかったら。
    //if (t <= 0.9f)
    //{
            
    //    // サンプリングした輝度をもとに、暗かったら星空を描画する。
    //    t = (1.0f - t);
    //    if (t != 0.0f)
    //    {
    //        t = pow(t, 10.0f);
    //        //t = pow(2.0f, 10.0f * t - 10.0f);
    //    }
    //    //payloadBuff.color_ += (float3) TexColor * t * payloadBuff.impactAmount_;
    //    //payloadBuff.color_ = saturate(payloadBuff.color_);
            
    //}
        
    // 影響度を0にする。
    payloadBuff.impactAmount_ = 0.0f;
        
    PayloadData = payloadBuff;

}

// シャドウ用missシェーダー
[shader("miss")]
void shadowMS(inout Payload payload)
{
    // 何にも当たっていないということなので、影は生成しない。
    payload.impactAmount_ = 1.0f;
}

// ライティング前処理
bool ProcessingBeforeLighting(inout Payload PayloadData, Vertex Vtx, MyAttribute Attrib, float3 WorldPos, float3 WorldNormal, inout float4 TexColor, uint InstanceID)
{
    
    // ペイロード受け取り用変数。
    Payload payloadBuff = PayloadData;
    
    // デノイズ用のマスクに使用するテクスチャに法線の色とInstanceIndexをかけたものを書き込む。
    if (payloadBuff.recursive_ == 1)
    {
        payloadBuff.denoiseMask_ = (WorldNormal);
    }
    
    // InstanceIDがCHS_IDENTIFICATION_INSTANCE_DEF_GI_TIREMASKだったらテクスチャに色を加算。
    if (InstanceID == CHS_IDENTIFICATION_INSTANCE_DEF_GI_TIREMASK || InstanceID == CHS_IDENTIFICATION_INSTANCE_DEF_TIREMASK)
    {
        float4 tiremasktex = (float4) tireMaskTexture[uint2((uint) (Vtx.subUV.x * 4096.0f), (uint) (Vtx.subUV.y * 4096.0f))];
        TexColor += tiremasktex * tiremasktex.a;
        TexColor = normalize(TexColor);
    }

    
    // 今発射されているレイのIDがGI用だったら
    if (payloadBuff.rayID_ == CHS_IDENTIFICATION_RAYID_GI)
    {
        
        // レイの長さを求める。
        float rayLength = length(WorldRayOrigin() - WorldPos);
        
        // GIを表示するレイの長さの最大値。
        const float MAX_RAY = 300.0f;
        
        // レイの長さの割合。
        float rate = rayLength / MAX_RAY;
        rate = 1.0f - saturate(rate);
        
        float3 giBuff = (float3) TexColor * rate * (1.0f - material[0].metalness_);

        
        payloadBuff.gi_ += giBuff;
        
        PayloadData = payloadBuff;
        
        return true;
    }
    
    // 当たったオブジェクトInstanceIDがテクスチャの色をそのまま返す or ライト用オブジェクトだったら
    if (InstanceID == CHS_IDENTIFICATION_INSTANCE_TEXCOLOR || InstanceID == CHS_IDENTIFICATION_INSTANCE_LIGHT)
    {
        payloadBuff.light_ += float3(1.0f, 1.0f, 1.0f) * payloadBuff.impactAmount_;
        payloadBuff.color_ += (float3) TexColor * payloadBuff.impactAmount_;
        payloadBuff.ao_ += 1.0f * payloadBuff.impactAmount_;
        payloadBuff.gi_ += float3(0.0f, 0.0f, 0.0f);
        
        // 影響度を0にする。
        payloadBuff.impactAmount_ = 0.0f;
        
        PayloadData = payloadBuff;
        
        return true;
    }
        
    PayloadData = payloadBuff;
    
    return false;
    
}

// UE4のGGX分布
float DistributionGGX(float Alpha, float NdotH)
{
    float alpha2 = Alpha * Alpha;
    float t = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * t * t);
}

// Schlickによるフレネルの近似 
float SchlickFresnel(float F0, float F90, float Cosine)
{
    float m = saturate(1.0f - Cosine);
    float m2 = m * m;
    float m5 = m2 * m2 * m;
    return lerp(F0, F90, m5);
}
float3 SchlickFresnel3(float3 F0, float3 F90, float Cosine)
{
    float m = saturate(1.0f - Cosine);
    float m2 = m * m;
    float m5 = m2 * m2 * m;
    return lerp(F0, F90, m5);
}

// ディズニーのフレネル計算
float3 DisneyFresnel(float LdotH, float3 BaseColor)
{
    
    // 輝度
    float luminance = 0.3f * BaseColor.r + 0.6f * BaseColor.g + 0.1f * BaseColor.b;
    // 色合い
    float3 tintColor = BaseColor / luminance;
    // 非金属の鏡面反射色を計算
    float3 nonMetalColor = material[0].specular_ * 0.08f * tintColor;
    // metalnessによる色補完 金属の場合はベースカラー
    float3 specularColor = lerp(nonMetalColor, BaseColor, material[0].metalness_);
    // NdotHの割合でSchlickFresnel補間
    return SchlickFresnel3(specularColor, float3(1.0f, 1.0f, 1.0f), LdotH);
    
}

// UE4のSmithモデル
float GeometricSmith(float Cosine)
{
    float k = (material[0].roughness_ + 1.0f);
    k = k * k / 8.0f;
    return Cosine / (Cosine * (1.0f - k) + k);
}

// 鏡面反射の計算
float3 CookTorranceSpecular(float NdotL, float NdotV, float NdotH, float LdotH, float3 BaseColor)
{
    
    // D項(分布:Distribution)
    float Ds = DistributionGGX(material[0].roughness_ * material[0].roughness_, NdotH);
    
    // F項(フレネル:Fresnel)
    float3 Fs = DisneyFresnel(LdotH, BaseColor);
    
    // G項(幾何減衰:Geometry attenuation)
    float Gs = GeometricSmith(NdotL) * GeometricSmith(NdotV);
    
    // M項(分母)
    float m = 4.0f * NdotL * NdotV;
    
    // 合成して鏡面反射の色を得る。
    return Ds * Fs * Gs / m;
    
}

// 双方向反射分布関数
float3 BRDF(float3 LightVec, float3 ViewVec, float3 Normal, float3 BaseColor)
{
    // 法線とライト方向の内積
    float NdotL = dot(Normal, LightVec);
    
    // 法線とカメラ方向の内積
    float NdotV = dot(Normal, ViewVec);
    
    // どちらかが90度以上であれば真っ黒を返す。
    if (NdotL < 0.0f || NdotV < 0.0f)
    {
        
        return float3(0.0f, 0.0f, 0.0f);
        
    }
    
    // ライト方向とカメラ方向の中間であるハーフベクトル
    float3 halfVec = normalize(LightVec + ViewVec);
    
    // 法線とハーフベクトルの内積
    float NdotH = dot(Normal, halfVec);
    
    // ライトとハーフベクトルの内積
    float LdotH = dot(LightVec, halfVec);
    
    // 拡散反射率
    float diffuseReflectance = 1.0f / PI;
    
    // 入射角が90度の場合の拡散反射率
    float energyBias = 0.5f * material[0].roughness_;
    float FD90 = energyBias + 2.0f * LdotH * LdotH * material[0].roughness_;
    
    // 入っていくときの拡散反射率
    float FL = SchlickFresnel(1.0f, FD90, NdotL);
    
    // 出ていくときの拡散反射率
    float FV = SchlickFresnel(1.0f, FD90, NdotV);
    
    // 入って出ていくまでの拡散反射率
    float energyFactor = lerp(1.0f, 1.0f / 1.51f, material[0].roughness_);
    float FD = FL * FV * energyFactor;
    
    // 拡散反射項
    float3 diffuseColor = diffuseReflectance * FD * BaseColor * (1.0f - material[0].metalness_);
    
    // 鏡面反射項
    float3 specularColor = CookTorranceSpecular(NdotL, NdotV, NdotH, LdotH, BaseColor);
    
    return diffuseColor + specularColor;
    
}

// ライティング処理
bool Lighting(inout Payload PayloadData, float3 WorldPos, float3 WorldNormal, Vertex Vtx, float4 TexColor)
{
    
    // Payload一時受け取り用変数。
    Payload payloadBuff = PayloadData;
    
    // 乱数の種となる値を取得。
    uint2 pixldx = DispatchRaysIndex().xy;
    uint2 numPix = DispatchRaysDimensions().xy;
    
    // 各光源の明るさ情報
    float aoLightVisibility = 0;
    
    // 点光源は一旦使ってないのでコメントアウト。後で戻す。
    
    //for (int index = 0; index < POINT_LIGHT_COUNT; ++index)
    //{
        
    //    // ポイントライトが有効化されていなかったら処理を飛ばす。
    //    if (!gSceneParam.light.pointLight[index].isActive)
    //    {
    //        continue;
    //    }
        
    //    // 光源までの長さ
    //    float lightLength = length(gSceneParam.light.pointLight[index].lightPos - WorldPos);
    
    //    // 光源までの長さが点光源の強さより小さかったら処理を飛ばす。
    //    if (lightLength < gSceneParam.light.pointLight[index].lightPower && gSceneParam.light.pointLight[index].isActive)
    //    {
            
    //        // ライトの隠蔽度
    //        float pointLightVisibilityBuff = 0;
    //        if (gSceneParam.light.pointLight[index].isShadow)
    //        {
    //            pointLightVisibilityBuff = SoftShadow(Vtx, gSceneParam.light.pointLight[index].lightSize, lightLength, index);
    //        }
        
    //        // ライトの明るさが一定以上だったらスペキュラーなどを計算する。
    //        if (0 < pointLightVisibilityBuff)
    //        {
                
    //            float3 pointLightDir = WorldPos - gSceneParam.light.pointLight[index].lightPos;
    //            pointLightDir = normalize(pointLightDir);
            
    //            // ライトまでの距離の割合
    //            float rate = lightLength / gSceneParam.light.pointLight[index].lightPower;
    //            rate = pow(rate, 5);
    //            rate = 1.0f - rate;
                

    //            payloadBuff.light_ += gSceneParam.light.pointLight[index].lightColor * BRDF(-pointLightDir, -WorldRayDirection(), WorldNormal, material[0].baseColor_) * rate * PayloadData.impactAmount_;

    //        }
        
    //    }
    //}
    
            
    // 太陽の位置とベクトル
    float3 sunPos = -gSceneParam.light.dirLight.lightDir * 300000.0f;
    float3 sunDir = normalize(sunPos - WorldPos);
    
    // ディレクショナルライトの色
    if (gSceneParam.light.dirLight.isActive && gSceneParam.light.dirLight.lightDir.y < 0.2f)
    {
        
        // ディレクショナルライトの方向にレイを飛ばす。
        float dirLightVisibility = ShootDirShadow(Vtx, 10000.0f);
        
        // ディレクショナルライトの明るさが一定以上だったら
        if (0.0f < dirLightVisibility)
        {
            
            const float SKYDOME_RADIUS = 15000.0f;
            const float SAMPLING_POS_Y = 0.0f;
            
            // 天球の色をサンプリング
            //float3 samplingVec = normalize(float3(1.0f, 0.1f, 0.1f)) * SKYDOME_RADIUS;
            float3 samplingVec = normalize(-gSceneParam.light.dirLight.lightDir * float3(1.0f, 0.0f, 1.0f)) * SKYDOME_RADIUS;
            samplingVec.y = 0.1f;
            
            // サンプリングするベクトル
            samplingVec.y = SAMPLING_POS_Y;
            samplingVec = normalize(samplingVec);
            
            // サンプリングする座標
            float3 samplingPos;
            samplingPos = samplingVec * SKYDOME_RADIUS;
            
            // 大気散乱の色
            float3 mieColor = float3(1, 1, 1);
            float3 skydomeColor = AtmosphericScattering(samplingPos, mieColor);
            
            payloadBuff.light_ += BRDF(-gSceneParam.light.dirLight.lightDir, -WorldRayDirection(), WorldNormal, float3(1, 1, 1)) * PayloadData.impactAmount_;
            payloadBuff.light_ = saturate(payloadBuff.light_);
            
        }
        
        
    }
    
    // AOの計算。 一定以上の距離の場合はAOの計算を行わない。
    if (1000.0f < RayTCurrent() || payloadBuff.rayID_ == CHS_IDENTIFICATION_RAYID_RECLECTION)
    {
        payloadBuff.ao_ += 0.2f * payloadBuff.impactAmount_;
    }
    else
    {
     
        //int seed = InitRand(DispatchRaysIndex().x + (WorldPos.x / 1000.0f) + DispatchRaysIndex().y * numPix.x, 100, 16);
        //float3 sampleDir = GetUniformHemisphereSample(seed, WorldNormal);
        
        //float aoLightVisibilityBuff = ShootAOShadowRay(WorldPos, sampleDir, 5, gRtScene);
        
        //float NoL = saturate(dot(WorldNormal, sampleDir));
        //float pdf = 1.0f / (2.0f * PI);
        //aoLightVisibility += aoLightVisibilityBuff;
        //aoLightVisibility = clamp(aoLightVisibility, 0.3f, 1.0f);
    
        //// ライトの総合隠蔽度を求める。
        //float aoVisibility = aoLightVisibility;
    
        //// 各色を設定。
        //payloadBuff.ao_ += aoVisibility * payloadBuff.impactAmount_;
        payloadBuff.ao_ += 0.2f * payloadBuff.impactAmount_;
        
    }
    
    
    PayloadData = payloadBuff;
    
    return false;
    
}

// ライティング後処理
void ProccessingAfterLighting(inout Payload PayloadData, Vertex Vtx, float3 WorldPos, float3 WorldNormal, float4 DefTexColor, inout float4 TexColor, uint InstanceID)
{
    
    // Payload一時受け取り用変数。
    Payload payloadBuff = PayloadData;
    
    // 当たったオブジェクトがGIを行うオブジェクトで、GIを行うフラグが立っていたら。
    if ((InstanceID == CHS_IDENTIFICATION_INSTANCE_DEF_GI || InstanceID == CHS_IDENTIFICATION_INSTANCE_DEF_GI_TIREMASK))
    {
        if (0.0f < payloadBuff.impactAmount_)
        {
            ShootGIRay(Vtx, 150, payloadBuff, WorldNormal);
            payloadBuff.gi_ = (payloadBuff.gi_ * payloadBuff.impactAmount_);
            payloadBuff.gi_ = saturate(payloadBuff.gi_);
            payloadBuff.color_.xyz += (float3) TexColor * payloadBuff.impactAmount_;
        
            payloadBuff.impactAmount_ = 0.0f;
        }
        
    }
        
    // 金属度
    float metalness = 1.0f - material[0].metalness_;
    
    // 当たったオブジェクトのInstanceIDがアルファだったら
    if (InstanceID == CHS_IDENTIFICATION_INSTANCE_ALPHA || InstanceID == CHS_IDENTIFICATION_INSTANCE_ADD)
    {
        
        // アルファ値を求める。
        int instanceIndex = InstanceIndex();
        float alpha = 0;
        for (int alphaIndex = 0; alphaIndex < ALPHA_DATA_COUNT; ++alphaIndex)
        {
            if (gSceneParam.alphaData_.alphaData_[alphaIndex].instanceIndex_ != instanceIndex)
            {
                continue;
            }
            alpha = gSceneParam.alphaData_.alphaData_[alphaIndex].alpha_;
            break;
        }
        
        // アルファブレンドだったら
        if (InstanceID == CHS_IDENTIFICATION_INSTANCE_ALPHA)
        {
        
            if (payloadBuff.impactAmount_ < alpha * TexColor.w)
            {
                payloadBuff.color_.xyz += (float3) TexColor * TexColor.w * payloadBuff.impactAmount_;
                payloadBuff.light_ += float3(1 * payloadBuff.impactAmount_, 1 * payloadBuff.impactAmount_, 1 * payloadBuff.impactAmount_);
                payloadBuff.impactAmount_ = 0.0f;

            }
            else
            {
                payloadBuff.color_.xyz += (float3) TexColor * TexColor.w * alpha;
                payloadBuff.light_ += float3(1 * alpha * TexColor.w, 1 * alpha * TexColor.w, 1 * alpha * TexColor.w);
                payloadBuff.impactAmount_ -= alpha * TexColor.w;
            }
            
        }
        // 加算合成だったら
        else if (InstanceID == CHS_IDENTIFICATION_INSTANCE_ADD)
        {
            
            payloadBuff.light_ = float3(TexColor.w, TexColor.w, TexColor.w);
            
            if (payloadBuff.impactAmount_ < alpha * TexColor.w)
            {
                payloadBuff.color_.xyz += (float3) TexColor * TexColor.w;
                payloadBuff.light_ += float3(1 * payloadBuff.impactAmount_, 1 * payloadBuff.impactAmount_, 1 * payloadBuff.impactAmount_);
                payloadBuff.impactAmount_ = 0.0f;

            }
            else
            {
                payloadBuff.color_.xyz += (float3) TexColor * TexColor.w;
                payloadBuff.light_ += float3(1 * alpha * TexColor.w, 1 * alpha * TexColor.w, 1 * alpha * TexColor.w);
                payloadBuff.impactAmount_ -= alpha * TexColor.w;
            }
        }
        
        // アルファが一定以下だったら。
        if (alpha < 0.5f)
        {
            ++payloadBuff.alphaCounter_;
            if (3 <= payloadBuff.alphaCounter_)
            {
                payloadBuff.isCullingAlpha_ = true;
            }
        }
            
        if (0.0f < payloadBuff.impactAmount_)
        {
                
            // 反射レイを飛ばす。
            ShootRay(CHS_IDENTIFICATION_RAYID_DEF, WorldPos, WorldRayDirection(), payloadBuff, gRtScene);
            
        }
        
    }
    // 当たったオブジェクトのInstanceIDが屈折だったら
    else if (InstanceID == CHS_IDENTIFICATION_INSTANCE_REFRACTION)
    {
        
        if (payloadBuff.impactAmount_ < metalness)
        {
            payloadBuff.color_.xyz += (float3) TexColor * payloadBuff.impactAmount_;
            payloadBuff.impactAmount_ = 0.0f;

        }
        else
        {
            payloadBuff.color_.xyz += (float3) TexColor * metalness;
            payloadBuff.impactAmount_ -= metalness;
        }

        float refractVal = 1.4f;
        float3 rayDir = float3(0, 0, 0);

        float nr = dot(WorldNormal, WorldRayDirection());
        if (nr < 0)
        {

            // 空気中->オブジェクト
            float eta = 1.0f / refractVal;
            rayDir = refract(WorldRayDirection(), WorldNormal, eta);

        }
        else
        {

            // オブジェクト->空気中
            float eta = refractVal / 1.0f;
            rayDir = refract(WorldRayDirection(), -WorldNormal, eta);
      
        }
            
        if (0.0f < payloadBuff.impactAmount_)
        {
                
            ShootRay(CHS_IDENTIFICATION_RAYID_REFRACTION, WorldPos, rayDir, payloadBuff, gRtScene);
            
        }

    }
    if (5000.0f < RayTCurrent())
    {
        
        payloadBuff.color_ = TexColor;
        payloadBuff.impactAmount_ = 0.0f;
        
    }
    // 反射の処理
    else
    {
        
        float metal = metalness;
        float rougness = material[0].roughness_;
        payloadBuff.roughnessOffset_ = rougness * 100.0f;
        
        // metalnessマップの取得。
        if (material[0].mapParam_ == MAP_SPECULAR)
        {
            // metalnessマップの色を取得。
            float3 metalnessMapColor = (float3) mapTexture.SampleLevel(smp, Vtx.uv, 0.0f);
            //metal = saturate((1.0f - metalnessMapColor.x));
        }
        
        if (payloadBuff.impactAmount_ < metal)
        {
            payloadBuff.color_.xyz += (float3) TexColor * payloadBuff.impactAmount_;
            payloadBuff.impactAmount_ = 0.0f;
        }
        else
        {
            payloadBuff.color_.xyz += (float3) TexColor * metal;
            payloadBuff.impactAmount_ -= metal;
            
            if (0.0f < payloadBuff.impactAmount_)
            {
                
                // 反射レイを飛ばす。
                ShootRay(CHS_IDENTIFICATION_RAYID_RECLECTION, WorldPos, reflect(WorldRayDirection(), WorldNormal), payloadBuff, gRtScene);
                
            }
        
        }
        
    }
    
    PayloadData = payloadBuff;
    
}

// closesthitシェーダー レイがヒットした時に呼ばれるシェーダー
[shader("closesthit")]

    void mainCHS
    (inout
    Payload payload, MyAttribute
    attrib)
{
    
    // 影用レイだったら。
    if (payload.rayID_ == CHS_IDENTIFICATION_RAYID_SHADOW)
    {
        
        // アルファのオブジェクトだったら
        if (InstanceID() == CHS_IDENTIFICATION_INSTANCE_ALPHA)
        {
            
            // テクスチャの色を取得。
            Vertex meshInfo[3];
            Vertex vtx = GetHitVertex(attrib, vertexBuffer, indexBuffer, meshInfo);
            float4 texColor = (float4) texture.SampleLevel(smp, vtx.uv, 0.0f);
        
            payload.impactAmount_ = texColor.a;
            
            return;
            
        }
           
        payload.impactAmount_ = 0.0f;
        
        
        return;
    }

    // 呼び出し回数が制限を超えないようにする。
    ++payload.recursive_;
    if (5 < payload.recursive_ || payload.impactAmount_ <= 0 || payload.isCullingAlpha_)
    {
        
        return;
    }
    
    Vertex meshInfo[3];
    Vertex vtx = GetHitVertex(attrib, vertexBuffer, indexBuffer, meshInfo);
    uint instanceID = InstanceID();
    float3 worldPos = mul(float4(vtx.Position, 1), ObjectToWorld4x3());
    float3 worldNormal = normalize(mul(vtx.Normal, (float3x3) ObjectToWorld4x3()));
    
    // MipLevel計算処理
    float2 ddxUV;
    float2 ddyUV;
    if (payload.rayID_ != CHS_IDENTIFICATION_RAYID_DEF)
    {
        
        // レイの発射ベクトルを求めるのに必要な変数たち
        matrix mtxViewInv = gSceneParam.camera.mtxViewInv;
        matrix mtxProjInv = gSceneParam.camera.mtxProjInv;
        float2 dims = float2(DispatchRaysDimensions().xy);
        float aspect = dims.x / dims.y;
        
        // 現在のレイからX+方向の発射ベクトル
        uint2 launchIndex = DispatchRaysIndex().xy + uint2(1, 0);
        float2 d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
        float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
        //float3 rayDirX = normalize(mul(mtxViewInv, float4(target.xyz, 0)).xyz);
        float3 rayDir = WorldRayDirection();
        float rotationAmountR = 0.0174533f / 30.0f;
        float3 rayDirX = normalize(mul(rayDir, float3x3(1, 0, 0,
                                               0, cos(rotationAmountR), -sin(rotationAmountR),
                                               0, sin(rotationAmountR), cos(rotationAmountR))));
        
        // 現在のレイからY+方向の発射ベクトル
        launchIndex -= uint2(1, -1);
        d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
        target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
        //float3 rayDirY = normalize(mul(mtxViewInv, float4(target.xyz, 0)).xyz);
        float3 rayDirY = normalize(mul(rayDir, float3x3(cos(rotationAmountR), 0, sin(rotationAmountR),
                                               0, 1, 0,
                                               -sin(rotationAmountR), 0, cos(rotationAmountR))));
        
        // レイの射出地点。
        float3 worldRayOrigin = WorldRayOrigin() + (RayTMin() * rayDir);
        
        // ベクトルXが平面に当たるまでの長さと衝突地点を求める。
        float lengthX = dot(-worldNormal, worldRayOrigin - worldPos) / dot(worldNormal, rayDirX);
        float3 impPosX = rayDirX * lengthX + worldRayOrigin;
        
        // ベクトルYが平面に当たるまでの長さと衝突地点を求める。
        float lengthY = dot(-worldNormal, worldRayOrigin - worldPos) / dot(worldNormal, rayDirY);
        float3 impPosY = rayDirY * lengthY + worldRayOrigin;
        
        // XYの重心座標を求める。
        float3 baryX = CalcVertexBarys(impPosX, meshInfo[0].Position, meshInfo[1].Position, meshInfo[2].Position);
        float3 baryY = CalcVertexBarys(impPosY, meshInfo[0].Position, meshInfo[1].Position, meshInfo[2].Position);
        
        // uvを求めて、その差分を取得する。
        float2 uvX = baryX.x * meshInfo[0].uv + baryX.y * meshInfo[1].uv + baryX.z * meshInfo[2].uv;
        float2 uvY = baryY.x * meshInfo[0].uv + baryY.y * meshInfo[1].uv + baryY.z * meshInfo[2].uv;
        ddxUV = abs(uvX - vtx.uv);
        ddyUV = abs(uvY - vtx.uv);
        
        
    }
    else
    {
        // レイの発射ベクトルを求めるのに必要な変数たち
        matrix mtxViewInv = gSceneParam.camera.mtxViewInv;
        matrix mtxProjInv = gSceneParam.camera.mtxProjInv;
        float2 dims = float2(DispatchRaysDimensions().xy);
        float aspect = dims.x / dims.y;
        
        // 現在のレイからX+方向の発射ベクトル
        uint2 launchIndex = DispatchRaysIndex().xy + uint2(1, 0);
        float2 d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
        float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
        float3 rayDirX = normalize(mul(mtxViewInv, float4(target.xyz, 0)).xyz);
        
        // 現在のレイからY+方向の発射ベクトル
        launchIndex -= uint2(1, -1);
        d = (launchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
        target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
        float3 rayDirY = normalize(mul(mtxViewInv, float4(target.xyz, 0)).xyz);
        
        // レイの射出地点。
        float3 worldRayOrigin = WorldRayOrigin() + (RayTMin() * WorldRayDirection());
        
        // ベクトルXが平面に当たるまでの長さと衝突地点を求める。
        float lengthX = dot(-worldNormal, worldRayOrigin - worldPos) / dot(worldNormal, rayDirX);
        float3 impPosX = rayDirX * lengthX + worldRayOrigin;
        
        // ベクトルYが平面に当たるまでの長さと衝突地点を求める。
        float lengthY = dot(-worldNormal, worldRayOrigin - worldPos) / dot(worldNormal, rayDirY);
        float3 impPosY = rayDirY * lengthY + worldRayOrigin;
        
        // XYの重心座標を求める。
        float3 baryX = CalcVertexBarys(impPosX, meshInfo[0].Position, meshInfo[1].Position, meshInfo[2].Position);
        float3 baryY = CalcVertexBarys(impPosY, meshInfo[0].Position, meshInfo[1].Position, meshInfo[2].Position);
        
        // uvを求めて、その差分を取得する。
        float2 uvX = baryX.x * meshInfo[0].uv + baryX.y * meshInfo[1].uv + baryX.z * meshInfo[2].uv;
        float2 uvY = baryX.x * meshInfo[0].uv + baryX.y * meshInfo[1].uv + baryX.z * meshInfo[2].uv;
        ddxUV = abs(uvX - vtx.uv);
        ddyUV = abs(uvY - vtx.uv);
        
    }

    // テクスチャの色を取得。
    float4 texColor = (float4) texture.SampleGrad(smp, vtx.uv, ddxUV * payload.roughnessOffset_, ddyUV * payload.roughnessOffset_);
    float4 defTexColor = texColor;
    
    // 追加のマップがAO用だったら
    if (material[0].mapParam_ == MAP_AO)
    {
        float mapColor = (float4) mapTexture.SampleGrad(smp, vtx.uv, ddxUV * payload.roughnessOffset_, ddyUV * payload.roughnessOffset_);
        texColor *= mapColor;
    }
    // 追加のマップが法線用だったら
    if (material[0].mapParam_ == MAP_NORMAL)
    {
        float normalColor = (float4) mapTexture.SampleGrad(smp, vtx.uv, ddxUV * payload.roughnessOffset_, ddyUV * payload.roughnessOffset_);
    }
    
    // ライティング前の処理を実行。----- 全反射オブジェクトやテクスチャの色をそのまま使うオブジェクトの色取得処理。
    if (ProcessingBeforeLighting(payload, vtx, attrib, worldPos, worldNormal, texColor, instanceID))
    {
        return;
    }
    

    // ライティングの処理
    if (instanceID != CHS_IDENTIFICATION_INSTANCE_ALPHA)
    {
        if (Lighting(payload, worldPos, worldNormal, vtx, texColor))
        {
            return;
        }
    }
    
    // ライティング後の処理を実行。 ----- ライティングの結果を合成する処理や反射や屈折等の再びレイを飛ばす必要があるオブジェクトの処理。
    ProccessingAfterLighting(payload, vtx, worldPos, worldNormal, defTexColor, texColor, instanceID);
    
}

// 影用CHS 使用していない。
[shader("closesthit")]

    void shadowCHS
    (inout
    Payload payload, MyAttribute
    attrib)
{
}

// AnyHitShader
[shader("anyhit")]

    void mainAnyHit
    (inout
    Payload payload, MyAttribute
    attrib)
{
        
    Vertex meshInfo[3];
    Vertex vtx = GetHitVertex(attrib, vertexBuffer, indexBuffer, meshInfo);
    float4 diffuse = texture.SampleLevel(smp, vtx.uv, 0);
    if (diffuse.w < 0.1f)
    {
        IgnoreHit();

    }
    
    int instanceID = InstanceID();
    
    // 当たったオブジェクトのInstanceIDがライト用or不可視のオブジェクトだったら当たり判定を棄却
    if (instanceID == CHS_IDENTIFICATION_INSTANCE_LIGHT || instanceID == CHS_IDENTIFICATION_INSTANCE_INVISIBILITY)
    {
        IgnoreHit();

    }
    
    // 当たったオブジェクトのInstanceIDがアルファだったら
    if (instanceID == CHS_IDENTIFICATION_INSTANCE_ALPHA || instanceID == CHS_IDENTIFICATION_INSTANCE_ADD)
    {
        
        // 一定以上薄いアルファ値のオブジェクトとあたっていたら。
        if (payload.isCullingAlpha_)
        {
            IgnoreHit();

        }
        
        if (payload.rayID_ == CHS_IDENTIFICATION_RAYID_SHADOW)
        {
            IgnoreHit();

        }
        
        // アルファ値を求める。
        int instanceIndex = InstanceIndex();
        float alpha = 0;
        for (int alphaIndex = 0; alphaIndex < ALPHA_DATA_COUNT; ++alphaIndex)
        {
            if (gSceneParam.alphaData_.alphaData_[alphaIndex].instanceIndex_ != instanceIndex)
            {
                continue;
            }
            alpha = gSceneParam.alphaData_.alphaData_[alphaIndex].alpha_;
            break;
        }
        
        // テクスチャの色を取得。
        float4 texColor = (float4) texture.SampleLevel(smp, vtx.uv, 0.0f);
        texColor.xyz *= texColor.w;
        
        alpha *= texColor.w;
        
        if (alpha <= 0.1f)
        {
            IgnoreHit();

        }
        
    }
    
}
