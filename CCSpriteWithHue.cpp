//
//  CCSpriteWithHue.cpp
//  HelloWorld
//
//  Created by Xiao Fei on 16/7/1.
//
//

#include "CCSpriteWithHue.h"

void xRotateMat(float mat[3][3], float rs, float rc);
void yRotateMat(float mat[3][3], float rs, float rc);
void zRotateMat(float mat[3][3], float rs, float rc);

void matrixMult(float a[3][3], float b[3][3], float c[3][3]);
void hueMatrix(GLfloat mat[3][3], float angle);
void premultiplyAlpha(GLfloat mat[3][3], float alpha);

// 不用管，什么也没做
CCSpriteWithHue::CCSpriteWithHue()
{
    
}

// 不用管，什么也没做
CCSpriteWithHue::~CCSpriteWithHue()
{
    
}

// 不用管，做了一些，但和原来一样
CCSpriteWithHue* CCSpriteWithHue::create(const std::string &filename)
{
    CCSpriteWithHue *sprite = new (std::nothrow)CCSpriteWithHue();
    if (sprite && sprite->initWithFile(filename))
    {
        sprite->autorelease();
        return sprite;
    }
    
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

// 不用管，和原来一样
bool CCSpriteWithHue::initWithTexture(Texture2D *texture, const Rect &rect, bool rotated)
{
    if (!Sprite::initWithTexture(texture, rect, rotated))
    {
        return false;
    }
    
    this->setupDefaultSettings();
    this->initShader();
    return true;
}

// 这个数值需要设置进去
void CCSpriteWithHue::setupDefaultSettings()
{
    this->m_hue = 0.0;
}

// 这个需要加进去，核心内容
void CCSpriteWithHue::initShader()
{
    // 原来的方法是加入fsh和vsh文件，修改为写死字符串，对外提供接口方便一些，只需要拷贝cpp和h文件即可
    const GLchar *vsh = "   attribute vec4 a_position;\n \
                            attribute vec2 a_texCoord;\n \
                            attribute vec4 a_color;\n \
                            varying vec2 v_texCoord;\n \
                            void main()\n \
                            {\n \
                                gl_Position = CC_PMatrix * a_position;\n \
                                v_texCoord = a_texCoord;\n \
                            }";
    
    const GLchar *fsh = "   varying vec2 v_texCoord;\n \
                            uniform mat3 u_hue;\n \
                            uniform float u_alpha;\n \
                            void main()\n \
                            {\n \
                                vec4 pixColor = texture2D(CC_Texture0, v_texCoord);\n \
                                vec3 rgbColor;\n \
                                rgbColor = u_hue * pixColor.rgb;\n \
                                gl_FragColor = vec4(rgbColor.r,rgbColor.g,rgbColor.b, pixColor.a * u_alpha);\n \
                            }";
    
    GLProgram * p = new GLProgram();
    this->setGLProgram(p);
    p->release();
    
    // 从文件获取的方法：p->initWithFilenames("hueChange.vsh", "hueChange.fsh"); 
    p->initWithByteArrays(vsh, fsh);
    
    p->link();
    p->updateUniforms();
    this->getUniformLocations();
    this->updateColor();
}

// 这个Sprite没有
void CCSpriteWithHue::getUniformLocations()
{
    m_hueLocation = glGetUniformLocation(this->getGLProgram()->getProgram(), "u_hue");
    m_alphaLocation = glGetUniformLocation(this->getGLProgram()->getProgram(), "u_alpha");
}

// 这个是核心内容，必须要有
void CCSpriteWithHue::updateColorMatrix()
{
    this->getGLProgram()->use();
    GLfloat mat[3][3];
    memset(mat, 0, sizeof(GLfloat)*9);
    hueMatrix(mat, m_hue);
    premultiplyAlpha(mat, this->alpha());
    glUniformMatrix3fv(m_hueLocation, 1, GL_FALSE, (GLfloat *)&mat);
}

// 更新透明度
void CCSpriteWithHue::updateAlpha()
{
    this->getGLProgram()->use();
    glUniform1f(m_alphaLocation, this->alpha());
}

// 设置透明度
GLfloat CCSpriteWithHue::alpha()
{
    return _displayedOpacity / 255.0f;
}

// 设置色相，小数单位，一周360度为2PI，可以正负
void CCSpriteWithHue::setHue(GLfloat _hue)
{
    m_hue = _hue;
    this->updateColorMatrix();
}

// 设置色相，整数单位，一周360度，可以正负
// setHueDegree(180)和setHue(M_PI)效果一样，setHueDegree可以使用美工给的色相数值(-180~180)
void CCSpriteWithHue::setHueDegree(GLfloat degree)
{
    GLfloat hue = degree / 180 * M_PI;
    this->setHue(hue);
}

// 更新颜色
void CCSpriteWithHue::updateColor()
{
    Sprite::updateColor();
    this->updateColorMatrix();
    this->updateAlpha();
}


#pragma mark -

void xRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = 1.0;
    mat[0][1] = 0.0;
    mat[0][2] = 0.0;
    
    mat[1][0] = 0.0;
    mat[1][1] = rc;
    mat[1][2] = rs;
    
    mat[2][0] = 0.0;
    mat[2][1] = -rs;
    mat[2][2] = rc;
}

void yRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = 0.0;
    mat[0][2] = -rs;
    
    mat[1][0] = 0.0;
    mat[1][1] = 1.0;
    mat[1][2] = 0.0;
    
    mat[2][0] = rs;
    mat[2][1] = 0.0;
    mat[2][2] = rc;
}


void zRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = rs;
    mat[0][2] = 0.0;
    
    mat[1][0] = -rs;
    mat[1][1] = rc;
    mat[1][2] = 0.0;
    
    mat[2][0] = 0.0;
    mat[2][1] = 0.0;
    mat[2][2] = 1.0;
}

void matrixMult(float a[3][3], float b[3][3], float c[3][3])
{
    int x, y;
    float temp[3][3];
    
    for (y = 0; y < 3; y++)
        for (x = 0; x < 3; x++)
            temp[y][x] = b[y][0] * a[0][x] + b[y][1] * a[1][x] + b[y][2] * a[2][x];
    
    for (y = 0; y < 3; y++)
        for (x = 0; x < 3; x ++)
            c[y][x] = temp[y][x];
}

void hueMatrix(GLfloat mat[3][3], float angle)
{
#define SQRT_2  sqrt(2.0)
#define SQRT_3  sqrt(3.0)
    
    float mag, rot[3][3];
    float xrs, xrc;
    float yrs, yrc;
    float zrs, zrc;
    
    mag = SQRT_2;
    xrs = 1.0 / mag;
    xrc = 1.0 / mag;
    xRotateMat(mat, xrs, xrc);
    
    mag = SQRT_3;
    yrs = -1.0 / mag;
    yrc = SQRT_2 / mag;
    yRotateMat(rot, yrs, yrc);
    
    matrixMult(rot, mat, mat);
    
    zrs = sin(angle);
    zrc = cos(angle);
    zRotateMat(rot, zrs, zrc);
    matrixMult(rot, mat, mat);
    
    
    yRotateMat(rot, -yrs, yrc);
    matrixMult(rot, mat, mat);
    xRotateMat(rot, -xrs, xrc);
    matrixMult(rot, mat, mat);
}

void premultiplyAlpha(GLfloat mat[3][3], float alpha)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mat[i][j] *= alpha;
}


