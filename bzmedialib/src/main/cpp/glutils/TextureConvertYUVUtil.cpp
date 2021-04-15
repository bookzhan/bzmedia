
/**
 * Created by bookzhan on 2017-08-02 17:09.
 * 说明:
 */

#include <common/BZLogUtil.h>
#include <mediaedit/VideoUtil.h>
#include <common/GLUtil.h>
#include "TextureConvertYUVUtil.h"
#include "../common/bzgl.h"
#include "../common/bz_time.h"
#include "TextureUtil.h"

bool TextureConvertYUVUtil::useHDShader = true;

int TextureConvertYUVUtil::init(int width, int height) {
    BZLogUtil::logD("TextureConvertYUVUtil init width=%d  height=%d", width, height);
    GLUtil::checkGlError(" TextureConvertYUVUtil::init before");

    width = width / 8 * 8;//取整
    height = height / 8 * 8;
    this->width = width;
    this->height = height;
    BZLogUtil::logD("TextureConvertYUVUtil init 对齐后 width=%d  height=%d", width, height);

    if (useHDShader) {
        BZLogUtil::logD("usetHighpShader");
        program = TextureUtil::initProgram(defaultVertexSource, defaultHighpFragmentSource);
    } else {
        BZLogUtil::logD("useMediumpShader");
        program = TextureUtil::initProgram(defaultVertexSource, defaultMediumpFragmentSource);
    }

    imageTextureLocation = (GLuint) glGetUniformLocation(program, "tex");
    widthLocation = (GLuint) glGetUniformLocation(program, "width");
    heightLocation = (GLuint) glGetUniformLocation(program, "height");

    vPositionLocation = glGetAttribLocation(program, "vPosition");
    inputTextureCoordinateLocation = glGetAttribLocation(program, "inputTextureCoordinate");

    glUseProgram(program);
    glUniform1f(widthLocation, width);
    glUniform1f(heightLocation, height);
    glUniform1i(imageTextureLocation, 0);

    glUseProgram(0);

    //coordinateBuffer
    glGenBuffers(1, &coordinateBuffer);
    //存放顶点位置数据
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TEXTURE_VERTICES),
                 TEXTURE_VERTICES,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //positionBuffer
    glGenBuffers(1, &positionBuffer);
    //存放顶点位置数据
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE),
                 CUBE, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //frameBuffer
    glGenFramebuffers(1, &frameBuffer);
    glGenTextures(1, &frameBuffersTextureId);
    glBindTexture(GL_TEXTURE_2D, frameBuffersTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, frameBuffersTextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLUtil::checkGlError(" TextureConvertYUVUtil::init end");
    return 0;
}

AVFrame *
TextureConvertYUVUtil::textureConvertYUV(GLint textureId) {
    glViewport(0, 0, width, height);
    //防止外面设置,导致GL_BLEND 状态出问题
    GLboolean glBlendIsEnabled = glIsEnabled(GL_BLEND);
    if (glBlendIsEnabled) {
        glDisable(GL_BLEND);
    }
    if (glIsProgram(program) != GL_TRUE) {
        BZLogUtil::logE("program not enable create a new");
        destroyResource();
        init(width, height);
    }
    if (glIsFramebuffer(frameBuffer) != GL_TRUE) {
        BZLogUtil::logE("frameBuffer not enable create a new");
        destroyResource();
        init(width, height);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glUseProgram(program);
    glEnableVertexAttribArray((GLuint) vPositionLocation);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer((GLuint) vPositionLocation, COORDS_PER_VERTEX, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE,
                          0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
    glBindBuffer(GL_ARRAY_BUFFER, coordinateBuffer);
    glVertexAttribPointer((GLuint) inputTextureCoordinateLocation, COORDS_PER_VERTEX, GL_FLOAT,
                          GL_FALSE, VERTEX_STRIDE, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, (GLuint) textureId);


//    glActiveTexture(GL_TEXTURE0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, drawOrder);


    int64_t startTime = getCurrentTime();
    AVFrame *temp = NULL;

    temp = captureYUVByDirectReadPix();

    totalTime += getCurrentTime() - startTime;
    count++;
//    BZLogUtil::logD("总体 耗时=%lld", getCurrentTime() - startTime);
    glDisableVertexAttribArray((GLuint) vPositionLocation);
    glDisableVertexAttribArray((GLuint) inputTextureCoordinateLocation);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);

    if (glBlendIsEnabled) {
        glEnable(GL_BLEND);
        //glBlendFunc(GL_ONE, GL_ONE);
    }
    return temp;
}

AVFrame *TextureConvertYUVUtil::captureYUVByDirectReadPix() {
    //Y
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    AVFrame *captureAVFrame = VideoUtil::allocVideoFrame(AV_PIX_FMT_YUV420P, width, height);

    glReadPixels(0, 0, width / 4, height,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, captureAVFrame->data[0]);
    //U
    glReadPixels(width / 4, 0, width / 8, height / 2,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, captureAVFrame->data[1]);

    //V
    glReadPixels(width / 4, height / 2, width / 8, height / 2,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE, captureAVFrame->data[2]);
    if (captureAVFrame->data[0] == NULL || captureAVFrame->data[1] == NULL ||
        captureAVFrame->data[2] == NULL) {
        BZLogUtil::logE("avFrame data is NULL");
        av_frame_free(&captureAVFrame);
    } else {
        return captureAVFrame;
    }
    return NULL;
}

int TextureConvertYUVUtil::destroyResource() {
    BZLogUtil::logD("TextureConvertYUVUtil::destroyResource()");
    BZLogUtil::logD("采集像素 平均耗时=%lld", totalTime / count);
    int64_t startTime = getCurrentTime();
    GLUtil::checkGlError("TextureConvertYUVUtil::destroyResource start");
    if (NULL != pixBuffers) {
        glDeleteBuffers(2, pixBuffers);
        pixBuffers = NULL;
    }

    //这个删除了会导致黑屏,原因不明
    if (frameBuffer > 0 && glIsFramebuffer(frameBuffer)) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }
    if (frameBuffersTextureId > 0 && glIsTexture(frameBuffersTextureId)) {
        glDeleteTextures(1, &frameBuffersTextureId);
        frameBuffersTextureId = 0;
    }
    if (positionBuffer > 0 && glIsBuffer(positionBuffer)) {
        glDeleteBuffers(1, &positionBuffer);
        positionBuffer = 0;
    }
    if (coordinateBuffer > 0 && glIsBuffer(coordinateBuffer)) {
        glDeleteBuffers(1, &coordinateBuffer);
        coordinateBuffer = 0;
    }
    BZLogUtil::logD("TextureConvertYUVUtil::finish 耗时=%lld", getCurrentTime() - startTime);
    GLUtil::checkGlError("TextureConvertYUVUtil::destroyResource end");
    return 0;
}


const char *TextureConvertYUVUtil::defaultVertexSource = "attribute vec4 vPosition;\n"
                                                         "attribute vec2 inputTextureCoordinate;\n"
                                                         "varying vec2 textureCoordinate;\n"
                                                         "void main()\n"
                                                         "{\n"
                                                         "    gl_Position = vPosition;\n"
                                                         "    textureCoordinate = inputTextureCoordinate;\n"
                                                         "}";
const char *TextureConvertYUVUtil::defaultHighpFragmentSource = "precision highp float;\n"
                                                                "varying vec2 textureCoordinate;//textureCoordinate;\n"
                                                                "uniform sampler2D tex;// 原始纹理\n"
                                                                "uniform float width;// 纹理宽\n"
                                                                "uniform float height;// 纹理高\n"
                                                                "\n"
                                                                "//图像宽度 *0.5 图像高度的范围内数据是正确的\n"
                                                                "\n"
                                                                "void main() {\n"
                                                                "\n"
                                                                "    vec2 samplingPos =vec2(0.0,0.0);\n"
                                                                "    vec4 texel=vec4(0.0,0.0,0.0,0.0);\n"
                                                                "\n"
                                                                "    vec3 offset = vec3(0.0625, 0.5, 0.5);\n"
                                                                "    //颜色系数矩阵 若输出颜色偏色可尝试交换ucoeff和vcoeff\n"
                                                                "    vec3 ycoeff = vec3(0.256816, 0.504154, 0.0979137);\n"
                                                                "    vec3 ucoeff = vec3(-0.148246, -0.29102, 0.439266);\n"
                                                                "    vec3 vcoeff = vec3(0.439271, -0.367833, -0.071438);\n"
                                                                "\n"
                                                                "\n"
                                                                "\n"
                                                                "    /* 公式\n"
                                                                "        mat4 RGBtoYUV(0.257,  0.439, -0.148, 0.0,\n"
                                                                "                     0.504, -0.368, -0.291, 0.0,\n"
                                                                "                     0.098, -0.071,  0.439, 0.0,\n"
                                                                "                     0.0625, 0.500,  0.500, 1.0 );\n"
                                                                "\n"
                                                                "        YUV = RGBtoYUV * RGB;\n"
                                                                "        */\n"
                                                                "    vec2 nowTxtPos = textureCoordinate;\n"
                                                                "    vec2 size = vec2(width, height);//1920 1080\n"
                                                                "\n"
                                                                "\n"
                                                                "    vec2 yScale = vec2(4,1);\n"
                                                                "    vec2 uvScale = vec2(8,2);\n"
                                                                "    vec2 hehe =vec2(0.0,0.0);\n"
                                                                "\n"
                                                                "    /*\n"
                                                                "        FBO总大小为w*h*4  转换后YUV420P 总共占w*h*3/2的空间\n"
                                                                "        则相对FBO总大小 u,v分量各占1/16   则 Y分量占1/16 *2 *2 = 1/4 =0.25 比例\n"
                                                                "\n"
                                                                "        */\n"
                                                                "    // y 占1/4\n"
                                                                "    if(nowTxtPos.x < 0.25){ //1\n"
                                                                "\n"
                                                                "        // y base postion\n"
                                                                "        vec2 basePos1 = (nowTxtPos * size +hehe) ; //  0.99996的情况？\n"
                                                                "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* yScale;//取整\n"
                                                                "        // vec2 basePos =vec2(basePos1.x,basePos1.y)* yScale;//取整\n"
                                                                "        // y1 y2 y3 y4\n"
                                                                "        float y1,y2,y3,y4;\n"
                                                                "\n"
                                                                "        //1\n"
                                                                "        samplingPos =  basePos / size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        y1 = dot(texel.rgb, ycoeff);\n"
                                                                "        y1 += offset.x;\n"
                                                                "\n"
                                                                "        //2\n"
                                                                "        basePos.x+=1.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        y2 = dot(texel.rgb, ycoeff);\n"
                                                                "        y2 += offset.x;\n"
                                                                "\n"
                                                                "        //3\n"
                                                                "        basePos.x+=1.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        y3 = dot(texel.rgb, ycoeff);\n"
                                                                "        y3 += offset.x;\n"
                                                                "\n"
                                                                "        //4\n"
                                                                "        basePos.x+=1.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        y4 = dot(texel.rgb, ycoeff);\n"
                                                                "        y4 += offset.x;\n"
                                                                "\n"
                                                                "        //写入亮度值\n"
                                                                "        gl_FragColor = vec4(y1, y2, y3, y4);\n"
                                                                "\n"
                                                                "    }\n"
                                                                "    //采集U\n"
                                                                "    else if(nowTxtPos.x >=0.25 && nowTxtPos.x < 0.375 && nowTxtPos.y < 0.5)\n"
                                                                "    {\n"
                                                                "        nowTxtPos.x -=0.25;//纹理坐标返回 (0,0)\n"
                                                                "        vec2 basePos1 = (nowTxtPos * size +hehe) ;\n"
                                                                "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* uvScale;//取整\n"
                                                                "        //得到像素坐标\n"
                                                                "        float v1,v2,v3,v4;\n"
                                                                "\n"
                                                                "        //1\n"
                                                                "        basePos.x+=0.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        v1 = dot(texel.rgb, vcoeff);\n"
                                                                "        v1 += offset.z;\n"
                                                                "        //2\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        v2 = dot(texel.rgb, vcoeff);\n"
                                                                "        v2 += offset.z;\n"
                                                                "        //3\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        v3 = dot(texel.rgb, vcoeff);\n"
                                                                "        v3 += offset.z;\n"
                                                                "        //4\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        v4 = dot(texel.rgb, vcoeff);\n"
                                                                "        v4 += offset.z;\n"
                                                                "\n"
                                                                "\n"
                                                                "        //写入V值\n"
                                                                "        gl_FragColor = vec4(v1, v2, v3, v4);\n"
                                                                "\n"
                                                                "    }\n"
                                                                "    //奇数行采集V\n"
                                                                "    else if(nowTxtPos.x >=0.25 && nowTxtPos.x < 0.375 && nowTxtPos.y >= 0.5)\n"
                                                                "    {\n"
                                                                "        nowTxtPos.x -=0.25;\n"
                                                                "        nowTxtPos.y -=0.5; //纹理坐标返回 (0,0)\n"
                                                                "\n"
                                                                "        vec2 basePos1 = (nowTxtPos * size +hehe) ;\n"
                                                                "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* uvScale;//取整\n"
                                                                "        //得到像素坐标\n"
                                                                "        float u1,u2,u3,u4;\n"
                                                                "\n"
                                                                "        //1\n"
                                                                "        basePos.x+=0.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        u1 = dot(texel.rgb, ucoeff);\n"
                                                                "        u1 += offset.y;\n"
                                                                "        //2\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        u2 = dot(texel.rgb, ucoeff);\n"
                                                                "        u2 += offset.y;\n"
                                                                "        //3\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        u3 = dot(texel.rgb, ucoeff);\n"
                                                                "        u3 += offset.y;\n"
                                                                "        //4\n"
                                                                "        basePos.x+=2.0;\n"
                                                                "        basePos.y+=0.0;\n"
                                                                "        samplingPos = basePos/size;\n"
                                                                "        texel = texture2D(tex, samplingPos);\n"
                                                                "        u4 = dot(texel.rgb, ucoeff);\n"
                                                                "        u4 += offset.y;\n"
                                                                "\n"
                                                                "        //写入U值\n"
                                                                "        gl_FragColor = vec4(u1, u2, u3, u4);\n"
                                                                "    }\n"
                                                                "    else\n"
                                                                "    {\n"
                                                                "        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
                                                                "    }\n"
                                                                "}";

const char *TextureConvertYUVUtil::defaultMediumpFragmentSource = "precision mediump float;\n"
                                                                  "varying vec2 textureCoordinate;//textureCoordinate;\n"
                                                                  "uniform sampler2D tex;// 原始纹理\n"
                                                                  "uniform float width;// 纹理宽\n"
                                                                  "uniform float height;// 纹理高\n"
                                                                  "\n"
                                                                  "//图像宽度 *0.5 图像高度的范围内数据是正确的\n"
                                                                  "\n"
                                                                  "void main() {\n"
                                                                  "\n"
                                                                  "    vec2 samplingPos =vec2(0.0,0.0);\n"
                                                                  "    vec4 texel=vec4(0.0,0.0,0.0,0.0);\n"
                                                                  "\n"
                                                                  "    vec3 offset = vec3(0.0625, 0.5, 0.5);\n"
                                                                  "    //颜色系数矩阵 若输出颜色偏色可尝试交换ucoeff和vcoeff\n"
                                                                  "    vec3 ycoeff = vec3(0.256816, 0.504154, 0.0979137);\n"
                                                                  "    vec3 ucoeff = vec3(-0.148246, -0.29102, 0.439266);\n"
                                                                  "    vec3 vcoeff = vec3(0.439271, -0.367833, -0.071438);\n"
                                                                  "\n"
                                                                  "\n"
                                                                  "\n"
                                                                  "    /* 公式\n"
                                                                  "        mat4 RGBtoYUV(0.257,  0.439, -0.148, 0.0,\n"
                                                                  "                     0.504, -0.368, -0.291, 0.0,\n"
                                                                  "                     0.098, -0.071,  0.439, 0.0,\n"
                                                                  "                     0.0625, 0.500,  0.500, 1.0 );\n"
                                                                  "\n"
                                                                  "        YUV = RGBtoYUV * RGB;\n"
                                                                  "        */\n"
                                                                  "    vec2 nowTxtPos = textureCoordinate;\n"
                                                                  "    vec2 size = vec2(width, height);//1920 1080\n"
                                                                  "\n"
                                                                  "\n"
                                                                  "    vec2 yScale = vec2(4,1);\n"
                                                                  "    vec2 uvScale = vec2(8,2);\n"
                                                                  "    vec2 hehe =vec2(0.0,0.0);\n"
                                                                  "\n"
                                                                  "    /*\n"
                                                                  "        FBO总大小为w*h*4  转换后YUV420P 总共占w*h*3/2的空间\n"
                                                                  "        则相对FBO总大小 u,v分量各占1/16   则 Y分量占1/16 *2 *2 = 1/4 =0.25 比例\n"
                                                                  "\n"
                                                                  "        */\n"
                                                                  "    // y 占1/4\n"
                                                                  "    if(nowTxtPos.x < 0.25){ //1\n"
                                                                  "\n"
                                                                  "        // y base postion\n"
                                                                  "        vec2 basePos1 = (nowTxtPos * size +hehe) ; //  0.99996的情况？\n"
                                                                  "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* yScale;//取整\n"
                                                                  "        // vec2 basePos =vec2(basePos1.x,basePos1.y)* yScale;//取整\n"
                                                                  "        // y1 y2 y3 y4\n"
                                                                  "        float y1,y2,y3,y4;\n"
                                                                  "\n"
                                                                  "        //1\n"
                                                                  "        samplingPos =  basePos / size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        y1 = dot(texel.rgb, ycoeff);\n"
                                                                  "        y1 += offset.x;\n"
                                                                  "\n"
                                                                  "        //2\n"
                                                                  "        basePos.x+=1.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        y2 = dot(texel.rgb, ycoeff);\n"
                                                                  "        y2 += offset.x;\n"
                                                                  "\n"
                                                                  "        //3\n"
                                                                  "        basePos.x+=1.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        y3 = dot(texel.rgb, ycoeff);\n"
                                                                  "        y3 += offset.x;\n"
                                                                  "\n"
                                                                  "        //4\n"
                                                                  "        basePos.x+=1.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        y4 = dot(texel.rgb, ycoeff);\n"
                                                                  "        y4 += offset.x;\n"
                                                                  "\n"
                                                                  "        //写入亮度值\n"
                                                                  "        gl_FragColor = vec4(y1, y2, y3, y4);\n"
                                                                  "\n"
                                                                  "    }\n"
                                                                  "    //采集U\n"
                                                                  "    else if(nowTxtPos.x >=0.25 && nowTxtPos.x < 0.375 && nowTxtPos.y < 0.5)\n"
                                                                  "    {\n"
                                                                  "        nowTxtPos.x -=0.25;//纹理坐标返回 (0,0)\n"
                                                                  "        vec2 basePos1 = (nowTxtPos * size +hehe) ;\n"
                                                                  "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* uvScale;//取整\n"
                                                                  "        //得到像素坐标\n"
                                                                  "        float v1,v2,v3,v4;\n"
                                                                  "\n"
                                                                  "        //1\n"
                                                                  "        basePos.x+=0.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        v1 = dot(texel.rgb, vcoeff);\n"
                                                                  "        v1 += offset.z;\n"
                                                                  "        //2\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        v2 = dot(texel.rgb, vcoeff);\n"
                                                                  "        v2 += offset.z;\n"
                                                                  "        //3\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        v3 = dot(texel.rgb, vcoeff);\n"
                                                                  "        v3 += offset.z;\n"
                                                                  "        //4\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        v4 = dot(texel.rgb, vcoeff);\n"
                                                                  "        v4 += offset.z;\n"
                                                                  "\n"
                                                                  "\n"
                                                                  "        //写入V值\n"
                                                                  "        gl_FragColor = vec4(v1, v2, v3, v4);\n"
                                                                  "\n"
                                                                  "    }\n"
                                                                  "    //奇数行采集V\n"
                                                                  "    else if(nowTxtPos.x >=0.25 && nowTxtPos.x < 0.375 && nowTxtPos.y >= 0.5)\n"
                                                                  "    {\n"
                                                                  "        nowTxtPos.x -=0.25;\n"
                                                                  "        nowTxtPos.y -=0.5; //纹理坐标返回 (0,0)\n"
                                                                  "\n"
                                                                  "        vec2 basePos1 = (nowTxtPos * size +hehe) ;\n"
                                                                  "        vec2 basePos =vec2(int(basePos1.x),int(basePos1.y))* uvScale;//取整\n"
                                                                  "        //得到像素坐标\n"
                                                                  "        float u1,u2,u3,u4;\n"
                                                                  "\n"
                                                                  "        //1\n"
                                                                  "        basePos.x+=0.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        u1 = dot(texel.rgb, ucoeff);\n"
                                                                  "        u1 += offset.y;\n"
                                                                  "        //2\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        u2 = dot(texel.rgb, ucoeff);\n"
                                                                  "        u2 += offset.y;\n"
                                                                  "        //3\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        u3 = dot(texel.rgb, ucoeff);\n"
                                                                  "        u3 += offset.y;\n"
                                                                  "        //4\n"
                                                                  "        basePos.x+=2.0;\n"
                                                                  "        basePos.y+=0.0;\n"
                                                                  "        samplingPos = basePos/size;\n"
                                                                  "        texel = texture2D(tex, samplingPos);\n"
                                                                  "        u4 = dot(texel.rgb, ucoeff);\n"
                                                                  "        u4 += offset.y;\n"
                                                                  "\n"
                                                                  "        //写入U值\n"
                                                                  "        gl_FragColor = vec4(u1, u2, u3, u4);\n"
                                                                  "    }\n"
                                                                  "    else\n"
                                                                  "    {\n"
                                                                  "        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
                                                                  "    }\n"
                                                                  "}";


