//
//  CCSpriteWithHue.hpp
//  HelloWorld
//
//  Created by Xiao Fei on 16/7/1.
//
//

#ifndef CCSpriteWithHue_h
#define CCSpriteWithHue_h

#include "cocos2d.h"
USING_NS_CC;

class CCSpriteWithHue : public Sprite
{
private:
    GLint m_hueLocation;
    GLint m_alphaLocation;
    
protected:
    
public:
    CC_SYNTHESIZE_READONLY(GLfloat, m_hue, Hue);
    
private:
    
protected:
    virtual void setupDefaultSettings();
    
    virtual void getUniformLocations();
    
    virtual void updateColorMatrix();
    
    virtual void updateAlpha();
    
    virtual GLfloat alpha();
    
    virtual void updateColor();
    
    virtual bool initWithTexture(Texture2D *texture, const Rect& rect, bool rotated);
    
    virtual void initShader();

public:
    
    CCSpriteWithHue();
    
    virtual ~CCSpriteWithHue();
    
    static CCSpriteWithHue* create(const std::string& filename);
    
    virtual void setHue(GLfloat _hue);
    
    virtual void setHueDegree(GLfloat Degree);

};

#endif /* CCSpriteWithHue_hpp */
