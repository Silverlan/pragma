// Define the cie_color_match table in GLSL as an array of vec3s
const vec3 cie_color_match[80] = vec3[](
    vec3(0.0014f, 0.0000f, 0.0065f), vec3(0.0022f, 0.0001f, 0.0105f), vec3(0.0042f, 0.0001f, 0.0201f),
    vec3(0.0076f, 0.0002f, 0.0362f), vec3(0.0143f, 0.0004f, 0.0679f), vec3(0.0232f, 0.0006f, 0.1102f),
    vec3(0.0435f, 0.0012f, 0.2074f), vec3(0.0776f, 0.0022f, 0.3713f), vec3(0.1344f, 0.0040f, 0.6456f),
    vec3(0.2148f, 0.0073f, 1.0391f), vec3(0.2839f, 0.0116f, 1.3856f), vec3(0.3285f, 0.0168f, 1.6230f),
    vec3(0.3483f, 0.0230f, 1.7471f), vec3(0.3481f, 0.0298f, 1.7826f), vec3(0.3362f, 0.0380f, 1.7721f),
    vec3(0.3187f, 0.0480f, 1.7441f), vec3(0.2908f, 0.0600f, 1.6692f), vec3(0.2511f, 0.0739f, 1.5281f),
    vec3(0.1954f, 0.0910f, 1.2876f), vec3(0.1421f, 0.1126f, 1.0419f), vec3(0.0956f, 0.1390f, 0.8130f),
    vec3(0.0580f, 0.1693f, 0.6162f), vec3(0.0320f, 0.2080f, 0.4652f), vec3(0.0147f, 0.2586f, 0.3533f),
    vec3(0.0049f, 0.3230f, 0.2720f), vec3(0.0024f, 0.4073f, 0.2123f), vec3(0.0093f, 0.5030f, 0.1582f),
    vec3(0.0291f, 0.6082f, 0.1117f), vec3(0.0633f, 0.7100f, 0.0782f), vec3(0.1096f, 0.7932f, 0.0573f),
    vec3(0.1655f, 0.8620f, 0.0422f), vec3(0.2257f, 0.9149f, 0.0298f), vec3(0.2904f, 0.9540f, 0.0203f),
    vec3(0.3597f, 0.9803f, 0.0134f), vec3(0.4334f, 0.9950f, 0.0087f), vec3(0.5121f, 1.0000f, 0.0057f),
    vec3(0.5945f, 0.9950f, 0.0039f), vec3(0.6784f, 0.9786f, 0.0027f), vec3(0.7621f, 0.9520f, 0.0021f),
    vec3(0.8425f, 0.9154f, 0.0018f), vec3(0.9163f, 0.8700f, 0.0017f), vec3(0.9786f, 0.8163f, 0.0014f),
    vec3(1.0263f, 0.7570f, 0.0011f), vec3(1.0567f, 0.6949f, 0.0010f), vec3(1.0622f, 0.6310f, 0.0008f),
    vec3(1.0456f, 0.5668f, 0.0006f), vec3(1.0026f, 0.5030f, 0.0003f), vec3(0.9384f, 0.4412f, 0.0002f),
    vec3(0.8544f, 0.3810f, 0.0002f), vec3(0.7514f, 0.3210f, 0.0001f), vec3(0.6424f, 0.2650f, 0.0000f),
    vec3(0.5419f, 0.2170f, 0.0000f), vec3(0.4479f, 0.1750f, 0.0000f), vec3(0.3608f, 0.1382f, 0.0000f),
    vec3(0.2835f, 0.1070f, 0.0000f), vec3(0.2187f, 0.0816f, 0.0000f), vec3(0.1649f, 0.0610f, 0.0000f),
    vec3(0.1212f, 0.0446f, 0.0000f), vec3(0.0874f, 0.0320f, 0.0000f), vec3(0.0636f, 0.0232f, 0.0000f),
    vec3(0.0468f, 0.0170f, 0.0000f), vec3(0.0329f, 0.0119f, 0.0000f), vec3(0.0227f, 0.0082f, 0.0000f),
    vec3(0.0158f, 0.0057f, 0.0000f), vec3(0.0114f, 0.0041f, 0.0000f), vec3(0.0081f, 0.0029f, 0.0000f),
    vec3(0.0058f, 0.0021f, 0.0000f), vec3(0.0041f, 0.0015f, 0.0000f), vec3(0.0029f, 0.0010f, 0.0000f),
    vec3(0.0020f, 0.0007f, 0.0000f), vec3(0.0014f, 0.0005f, 0.0000f), vec3(0.0010f, 0.0004f, 0.0000f),
    vec3(0.0007f, 0.0002f, 0.0000f), vec3(0.0005f, 0.0002f, 0.0000f), vec3(0.0003f, 0.0001f, 0.0000f),
    vec3(0.0002f, 0.0001f, 0.0000f), vec3(0.0002f, 0.0001f, 0.0000f), vec3(0.0001f, 0.0000f, 0.0000f),
    vec3(0.0001f, 0.0000f, 0.0000f), vec3(0.0001f, 0.0000f, 0.0000f), vec3(0.0000f, 0.0000f, 0.0000f)
);


ccl_device float3 xyz_to_rgb(float3 xyz)
{
  return make_float3(dot(float4_to_float3(kernel_data.film.xyz_to_r), xyz),
                     dot(float4_to_float3(kernel_data.film.xyz_to_g), xyz),
                     dot(float4_to_float3(kernel_data.film.xyz_to_b), xyz));
}

vec3 interp(vec3 a, vec3 b, float t) {
    return mix(a, b, t); // GLSL built-in linear interpolation
}

void svm_node_wavelength(float wavelength, out vec3 color_out) {
    float lambda_nm = wavelength;
    float ii = (lambda_nm - 380.0) * (1.0 / 5.0);  // scaled 0..80
    int i = int(ii);
    vec3 color;

    if (i < 0 || i >= 80) {
        color = vec3(0.0, 0.0, 0.0);
    } else {
        ii -= float(i);
        vec3 c0 = vec3(cie_color_match[i][0], cie_color_match[i][1], cie_color_match[i][2]);
        vec3 c1 = vec3(cie_color_match[i][3], cie_color_match[i][4], cie_color_match[i][5]);
        color = interp(c0, c1, ii);
    }

    color = xyz_to_rgb(color);
    color *= 1.0 / 2.52; // Empirical scale from lg to make all comps <= 1

    // Clamp to zero if values are smaller
    color = max(color, vec3(0.0, 0.0, 0.0));

    color_out = color;
}
