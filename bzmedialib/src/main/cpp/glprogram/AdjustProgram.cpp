//
/**
 * Created by bookzhan on 2021-03-11 13:58.
 *description:
 */
//

#include <GLES2/gl2.h>
#include "AdjustProgram.h"

AdjustProgram::AdjustProgram() {
    FRAGMENT_SHADER = "varying highp vec2 textureCoordinate;\n"
                      "uniform sampler2D inputImageTexture;\n"
                      "uniform lowp float shadows;\n"
                      "const mediump vec3 hsLuminanceWeighting = vec3(0.3, 0.3, 0.3);\n"
                      "uniform lowp float highlights;\n"
                      "uniform lowp float contrast;\n"
                      "uniform lowp float fadeAmount;\n"
                      "const mediump vec3 satLuminanceWeighting = vec3(0.2126, 0.7152, 0.0722);\n"
                      "uniform lowp float saturation;\n"
                      "uniform lowp float shadowsTintIntensity;\n"
                      "uniform lowp float highlightsTintIntensity;\n"
                      "uniform lowp vec3 shadowsTintColor;\n"
                      "uniform lowp vec3 highlightsTintColor;\n"
                      "uniform lowp float exposure;\n"
                      "uniform lowp float warmth;\n"
                      "uniform lowp float green;\n"
                      "uniform mediump float hueAdjust;\n"
                      "const lowp float permTexUnit = 1.0 / 256.0;\n"
                      "const lowp float permTexUnitHalf = 0.5 / 256.0;\n"
                      "uniform lowp float vignette;\n"
                      "const highp vec4 kRGBToYPrime = vec4(0.299, 0.587, 0.114, 0.0);\n"
                      "const highp vec4 kRGBToI = vec4(0.595716, -0.274453, -0.321263, 0.0);\n"
                      "const highp vec4 kRGBToQ = vec4(0.211456, -0.522591, 0.31135, 0.0);\n"
                      "const highp vec4 kYIQToR = vec4(1.0, 0.9563, 0.6210, 0.0);\n"
                      "const highp vec4 kYIQToG = vec4(1.0, -0.2721, -0.6474, 0.0);\n"
                      "const highp vec4 kYIQToB = vec4(1.0, -1.1070, 1.7046, 0.0);\n"
                      "\n"
                      "highp float getLuma(highp vec3 rgbP) {\n"
                      "    return (0.299 * rgbP.r) + (0.587 * rgbP.g) + (0.114 * rgbP.b);\n"
                      "}\n"
                      "\n"
                      "lowp vec3 rgbToHsv(lowp vec3 c) {\n"
                      "    highp vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
                      "    highp vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);\n"
                      "    highp vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);\n"
                      "    highp float d = q.x - min(q.w, q.y);\n"
                      "    highp float e = 1.0e-10;\n"
                      "    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n"
                      "}\n"
                      "\n"
                      "lowp vec3 hsvToRgb(lowp vec3 c) {\n"
                      "    highp vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
                      "    highp vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
                      "    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n"
                      "}\n"
                      "\n"
                      "highp vec3 rgbToHsl(highp vec3 color) {\n"
                      "    highp vec3 hsl;\n"
                      "    highp float fmin = min(min(color.r, color.g), color.b);\n"
                      "    highp float fmax = max(max(color.r, color.g), color.b);\n"
                      "    highp float delta = fmax - fmin;\n"
                      "    hsl.z = (fmax + fmin) / 2.0;\n"
                      "    if (delta == 0.0) {\n"
                      "        hsl.x = 0.0;\n"
                      "        hsl.y = 0.0;\n"
                      "    } else {\n"
                      "        if (hsl.z < 0.5) {\n"
                      "            hsl.y = delta / (fmax + fmin);\n"
                      "        } else {\n"
                      "            hsl.y = delta / (2.0 - fmax - fmin);\n"
                      "        }\n"
                      "        highp float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;\n"
                      "        highp float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;\n"
                      "        highp float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;\n"
                      "        if (color.r == fmax) {\n"
                      "            hsl.x = deltaB - deltaG;\n"
                      "        } else if (color.g == fmax) {\n"
                      "            hsl.x = (1.0 / 3.0) + deltaR - deltaB;\n"
                      "        } else if (color.b == fmax) {\n"
                      "            hsl.x = (2.0 / 3.0) + deltaG - deltaR;\n"
                      "        }\n"
                      "        if (hsl.x < 0.0) {\n"
                      "            hsl.x += 1.0;\n"
                      "        } else if (hsl.x > 1.0) {\n"
                      "            hsl.x -= 1.0;\n"
                      "        }\n"
                      "    }\n"
                      "    return hsl;\n"
                      "}\n"
                      "\n"
                      "highp float hueToRgb(highp float f1, highp float f2, highp float hue) {\n"
                      "    if (hue < 0.0) {\n"
                      "        hue += 1.0;\n"
                      "    } else if (hue > 1.0) {\n"
                      "        hue -= 1.0;\n"
                      "    }\n"
                      "    highp float res;\n"
                      "    if ((6.0 * hue) < 1.0) {\n"
                      "        res = f1 + (f2 - f1) * 6.0 * hue;\n"
                      "    } else if ((2.0 * hue) < 1.0) {\n"
                      "        res = f2;\n"
                      "    } else if ((3.0 * hue) < 2.0) {\n"
                      "        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;\n"
                      "    } else {\n"
                      "        res = f1;\n"
                      "    }\n"
                      "    return res;\n"
                      "}\n"
                      "\n"
                      "void colorHue(inout lowp vec4 color, mediump float hueVal) {\n"
                      "    // Convert to YIQ+\n"
                      "    highp float YPrime = dot(color, kRGBToYPrime);\n"
                      "    highp float I = dot(color, kRGBToI);\n"
                      "    highp float Q = dot(color, kRGBToQ);\n"
                      "    // Calculate the hue and chroma\n"
                      "    highp float hue = atan(Q, I);\n"
                      "    highp float chroma = sqrt(I * I + Q * Q);\n"
                      "    // Make the user's adjustments\n"
                      "    hue += (-hueVal); //why negative rotation?\n"
                      "    // Convert back to YIQ\n"
                      "    Q = chroma * sin(hue);\n"
                      "    I = chroma * cos(hue);\n"
                      "    // Convert back to RGB\n"
                      "    highp vec4 yIQ = vec4(YPrime, I, Q, 0.0);\n"
                      "    color.r = dot(yIQ, kYIQToR);\n"
                      "    color.g = dot(yIQ, kYIQToG);\n"
                      "    color.b = dot(yIQ, kYIQToB);\n"
                      "}\n"
                      "\n"
                      "highp vec3 hslToRgb(highp vec3 hsl) {\n"
                      "    if (hsl.y == 0.0) {\n"
                      "        return vec3(hsl.z);\n"
                      "    } else {\n"
                      "        highp float f2;\n"
                      "        if (hsl.z < 0.5) {\n"
                      "            f2 = hsl.z * (1.0 + hsl.y);\n"
                      "        } else {\n"
                      "            f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);\n"
                      "        }\n"
                      "        highp float f1 = 2.0 * hsl.z - f2;\n"
                      "        return vec3(hueToRgb(f1, f2, hsl.x + (1.0 / 3.0)), hueToRgb(f1, f2, hsl.x), hueToRgb(f1, f2, hsl.x - (1.0 / 3.0)));\n"
                      "    }\n"
                      "}\n"
                      "\n"
                      "highp vec3 rgbToYuv(highp vec3 inP) {\n"
                      "    highp float luma = getLuma(inP);\n"
                      "    return vec3(luma, (1.0 / 1.772) * (inP.b - luma), (1.0 / 1.402) * (inP.r - luma));\n"
                      "}\n"
                      "\n"
                      "lowp vec3 yuvToRgb(highp vec3 inP) {\n"
                      "    return vec3(1.402 * inP.b + inP.r, (inP.r - (0.299 * 1.402 / 0.587) * inP.b - (0.114 * 1.772 / 0.587) * inP.g), 1.772 * inP.g + inP.r);\n"
                      "}\n"
                      "\n"
                      "lowp float easeInOutSigmoid(lowp float value, lowp float strength) {\n"
                      "    if (value > 0.5) {\n"
                      "        return 1.0 - pow(2.0 - 2.0 * value, 1.0 / (1.0 - strength)) * 0.5;\n"
                      "    } else {\n"
                      "        return pow(2.0 * value, 1.0 / (1.0 - strength)) * 0.5;\n"
                      "    }\n"
                      "}\n"
                      "\n"
                      "highp vec3 fadeAdjust(highp vec3 color, highp float fadeVal) {\n"
                      "    return (color * (1.0 - fadeVal)) + ((color + (vec3(-0.9772) * pow(vec3(color), vec3(3.0)) + vec3(1.708) * pow(vec3(color), vec3(2.0)) + vec3(-0.1603) * vec3(color) + vec3(0.2878) - color * vec3(0.9))) * fadeVal);\n"
                      "}\n"
                      "\n"
                      "lowp vec3 tintRaiseShadowsCurve(lowp vec3 color) {\n"
                      "    return vec3(-0.003671) * pow(color, vec3(3.0)) + vec3(0.3842) * pow(color, vec3(2.0)) + vec3(0.3764) * color + vec3(0.2515);\n"
                      "}\n"
                      "\n"
                      "lowp vec3 tintShadows(lowp vec3 texel, lowp vec3 tintColor, lowp float tintAmount) {\n"
                      "    return clamp(mix(texel, mix(texel, tintRaiseShadowsCurve(texel), tintColor), tintAmount), 0.0, 1.0);\n"
                      "}\n"
                      "\n"
                      "lowp vec3 tintHighlights(lowp vec3 texel, lowp vec3 tintColor, lowp float tintAmount) {\n"
                      "    return clamp(mix(texel, mix(texel, vec3(1.0) - tintRaiseShadowsCurve(vec3(1.0) - texel), (vec3(1.0) - tintColor)), tintAmount), 0.0, 1.0);\n"
                      "}\n"
                      "\n"
                      "highp vec4 rnm( in highp vec2 tc) {\n"
                      "    highp float noise = sin(dot(tc, vec2(12.9898, 78.233))) * 43758.5453;\n"
                      "    return vec4(fract(noise), fract(noise * 1.2154), fract(noise * 1.3453), fract(noise * 1.3647)) * 2.0 - 1.0;\n"
                      "}\n"
                      "\n"
                      "highp float fade( in highp float t) {\n"
                      "    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);\n"
                      "}\n"
                      "\n"
                      "void main() {\n"
                      "    lowp vec4 source = texture2D(inputImageTexture, textureCoordinate);\n"
                      "    lowp vec4 result = source;\n"
                      "    const lowp float toolEpsilon = 0.005;\n"
                      "\n"
                      "    result = vec4(clamp(((result.rgb - vec3(0.5)) * contrast + vec3(0.5)), 0.0, 1.0), result.a);\n"
                      "\n"
                      "    if (abs(fadeAmount) > toolEpsilon) {\n"
                      "        result.rgb = fadeAdjust(result.rgb, fadeAmount);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(shadowsTintIntensity) > toolEpsilon) {\n"
                      "        result.rgb = tintShadows(result.rgb, shadowsTintColor, shadowsTintIntensity * 2.0);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(highlightsTintIntensity) > toolEpsilon) {\n"
                      "        result.rgb = tintHighlights(result.rgb, highlightsTintColor, highlightsTintIntensity * 2.0);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(highlights - 1.0) > toolEpsilon || abs(shadows - 1.0) > toolEpsilon) {\n"
                      "        mediump float hsLuminance = dot(result.rgb, hsLuminanceWeighting);\n"
                      "        mediump float shadow = clamp((pow(hsLuminance, 1.0 / shadows) + (-0.76) * pow(hsLuminance, 2.0 / shadows)) - hsLuminance, 0.0, 1.0);\n"
                      "        mediump float highlight = clamp((1.0 - (pow(1.0 - hsLuminance, 1.0 / (2.0 - highlights)) + (-0.8) * pow(1.0 - hsLuminance, 2.0 / (2.0 - highlights)))) - hsLuminance, -1.0, 0.0);\n"
                      "        lowp vec3 hsresult = vec3(0.0, 0.0, 0.0) + ((hsLuminance + shadow + highlight) - 0.0) * ((result.rgb - vec3(0.0, 0.0, 0.0)) / (hsLuminance - 0.0));\n"
                      "        mediump float contrastedLuminance = ((hsLuminance - 0.5) * 1.5) + 0.5;\n"
                      "        mediump float whiteInterp = contrastedLuminance * contrastedLuminance * contrastedLuminance;\n"
                      "        mediump float whiteTarget = clamp(highlights, 1.0, 2.0) - 1.0;\n"
                      "        hsresult = mix(hsresult, vec3(1.0), whiteInterp * whiteTarget);\n"
                      "        mediump float invContrastedLuminance = 1.0 - contrastedLuminance;\n"
                      "        mediump float blackInterp = invContrastedLuminance * invContrastedLuminance * invContrastedLuminance;\n"
                      "        mediump float blackTarget = 1.0 - clamp(shadows, 0.0, 1.0);\n"
                      "        hsresult = mix(hsresult, vec3(0.0), blackInterp * blackTarget);\n"
                      "        result = vec4(hsresult.rgb, result.a);\n"
                      "    }\n"
                      "\n"
                      "    lowp float satLuminance = dot(result.rgb, satLuminanceWeighting);\n"
                      "    lowp vec3 greyScaleColor = vec3(satLuminance);\n"
                      "    result = vec4(clamp(mix(greyScaleColor, result.rgb, saturation), 0.0, 1.0), result.a);\n"
                      "\n"
                      "    if (abs(exposure) > toolEpsilon) {\n"
                      "        mediump float mag = exposure * 1.045;\n"
                      "        mediump float exppower = 1.0 + abs(mag);\n"
                      "        if (mag < 0.0) {\n"
                      "            exppower = 1.0 / exppower;\n"
                      "        }\n"
                      "        result.r = 1.0 - pow((1.0 - result.r), exppower);\n"
                      "        result.g = 1.0 - pow((1.0 - result.g), exppower);\n"
                      "        result.b = 1.0 - pow((1.0 - result.b), exppower);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(warmth) > toolEpsilon) {\n"
                      "        highp vec3 yuvVec;\n"
                      "        if (warmth > 0.0) {\n"
                      "            yuvVec = vec3(0.1765, -0.1255, 0.0902);\n"
                      "        } else {\n"
                      "            yuvVec = -vec3(0.0588, 0.1569, -0.1255);\n"
                      "        }\n"
                      "        highp vec3 yuvColor = rgbToYuv(result.rgb);\n"
                      "        highp float luma = yuvColor.r;\n"
                      "        highp float curveScale = sin(luma * 3.14159);\n"
                      "        yuvColor += 0.375 * warmth * curveScale * yuvVec;\n"
                      "        result.rgb = yuvToRgb(yuvColor);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(green - 1.0) > toolEpsilon) {\n"
                      "        result = vec4(result.r, result.g * green, result.b, result.a);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(hueAdjust) > toolEpsilon) {\n"
                      "        colorHue(result, hueAdjust);\n"
                      "    }\n"
                      "\n"
                      "    if (abs(vignette) > toolEpsilon) {\n"
                      "        const lowp float midpoint = 0.7;\n"
                      "        const lowp float fuzziness = 0.62;\n"
                      "        lowp float radDist = length(textureCoordinate - 0.5) / sqrt(0.5);\n"
                      "        lowp float mag = easeInOutSigmoid(radDist * midpoint, fuzziness) * vignette * 0.645;\n"
                      "        result.rgb = mix(pow(result.rgb, vec3(1.0 / (1.0 - mag))), vec3(0.0), mag * mag);\n"
                      "    }\n"
                      "    gl_FragColor = result;\n"
                      "}";
    adjustEffectConfig = new AdjustEffectConfig();
    adjustEffectConfig->shadowsTintColor = new BZColor();
    adjustEffectConfig->highlightsTintColor = new BZColor();
}

void AdjustProgram::setAdjustConfig(AdjustEffectConfig *adjustEffectConfig) {
    if (nullptr == adjustEffectConfig) {
        return;
    }
    releaseAdjustEffectConfig(this->adjustEffectConfig);
    this->adjustEffectConfig = adjustEffectConfig;
}


int AdjustProgram::initProgram(const char *vertexShader, const char *fragmentShader) {
    GLuint program = static_cast<GLuint>(BaseProgram::initProgram(vertexShader, fragmentShader));
    shadowsLoc = glGetUniformLocation(program, "shadows");
    highlightsLoc = glGetUniformLocation(program, "highlights");
    contrastLoc = glGetUniformLocation(program, "contrast");
    fadeAmountLoc = glGetUniformLocation(program, "fadeAmount");
    saturationLoc = glGetUniformLocation(program, "saturation");
    shadowsTintIntensityLoc = glGetUniformLocation(program, "shadowsTintIntensity");
    highlightsTintIntensityLoc = glGetUniformLocation(program, "highlightsTintIntensity");
    shadowsTintColorLoc = glGetUniformLocation(program, "shadowsTintColor");
    highlightsTintColorLoc = glGetUniformLocation(program, "highlightsTintColor");
    exposureLoc = glGetUniformLocation(program, "exposure");
    warmthLoc = glGetUniformLocation(program, "warmth");
    greenLoc = glGetUniformLocation(program, "green");
    hueAdjustLoc = glGetUniformLocation(program, "hueAdjust");
    vignetteLoc = glGetUniformLocation(program, "vignette");
    return program;
}

int AdjustProgram::drawArraysBefore() {
    int ret = BaseProgram::drawArraysBefore();
    if (nullptr == adjustEffectConfig) {
        return ret;
    }
    glUniform1f(shadowsLoc, adjustEffectConfig->shadows);
    glUniform1f(highlightsLoc, adjustEffectConfig->highlights);
    glUniform1f(contrastLoc, adjustEffectConfig->contrast);
    glUniform1f(fadeAmountLoc, adjustEffectConfig->fadeAmount);
    glUniform1f(saturationLoc, adjustEffectConfig->saturation);
    glUniform1f(shadowsTintIntensityLoc, adjustEffectConfig->shadowsTintIntensity);
    glUniform1f(highlightsTintIntensityLoc, adjustEffectConfig->highlightsTintIntensity);
    if (nullptr != adjustEffectConfig->shadowsTintColor) {
        glUniform3f(shadowsTintColorLoc, adjustEffectConfig->shadowsTintColor->r,
                    adjustEffectConfig->shadowsTintColor->g,
                    adjustEffectConfig->shadowsTintColor->b);
    }
    if (nullptr != adjustEffectConfig->highlightsTintColor) {
        glUniform3f(highlightsTintColorLoc, adjustEffectConfig->highlightsTintColor->r,
                    adjustEffectConfig->highlightsTintColor->g,
                    adjustEffectConfig->highlightsTintColor->b);
    }
    glUniform1f(exposureLoc, adjustEffectConfig->exposure);
    glUniform1f(warmthLoc, adjustEffectConfig->warmth);
    glUniform1f(greenLoc, adjustEffectConfig->green);
    glUniform1f(hueAdjustLoc, adjustEffectConfig->hueAdjust);
    glUniform1f(vignetteLoc, adjustEffectConfig->vignette);
    return ret;
}

void AdjustProgram::releaseAdjustEffectConfig(AdjustEffectConfig *adjustEffectConfig) {
    if (nullptr == adjustEffectConfig) {
        return;
    }
    if (nullptr != adjustEffectConfig->shadowsTintColor) {
        delete (adjustEffectConfig->shadowsTintColor);
        adjustEffectConfig->shadowsTintColor = nullptr;
    }
    if (nullptr != adjustEffectConfig->highlightsTintColor) {
        delete (adjustEffectConfig->highlightsTintColor);
        adjustEffectConfig->highlightsTintColor = nullptr;
    }
    delete adjustEffectConfig;
}

int AdjustProgram::releaseResource() {
    if (nullptr != this->adjustEffectConfig) {
        releaseAdjustEffectConfig(this->adjustEffectConfig);
        this->adjustEffectConfig = nullptr;
    }
    return BaseProgram::releaseResource();
}
