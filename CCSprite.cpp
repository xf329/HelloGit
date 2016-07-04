/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "2d/CCSprite.h"

#include <algorithm>

#include "2d/CCSpriteBatchNode.h"
#include "2d/CCAnimationCache.h"
#include "2d/CCSpriteFrame.h"
#include "2d/CCSpriteFrameCache.h"
#include "renderer/CCTextureCache.h"
#include "renderer/CCTexture2D.h"
#include "renderer/CCRenderer.h"
#include "base/CCDirector.h"

#include "deprecated/CCString.h"


NS_CC_BEGIN

#if CC_SPRITEBATCHNODE_RENDER_SUBPIXEL
#define RENDER_IN_SUBPIXEL
#else
#define RENDER_IN_SUBPIXEL(__ARGS__) (ceil(__ARGS__))
#endif

bool
InitializeListHead (LIST_ENTRY *List)
{
    List->FLink = List;
    List->BLink = List;
    return true;
}

bool IsListEmpty(LIST_ENTRY *List)
{
    return (List->FLink == List);
}

void RemoveEntryList(LIST_ENTRY *Entry)
{
    LIST_ENTRY  *_ForwardLink;
    LIST_ENTRY  *_BackLink;
    
    _ForwardLink        = Entry->FLink;
    _BackLink           = Entry->BLink;
    _BackLink->FLink    = _ForwardLink;
    _ForwardLink->BLink = _BackLink;
}

void InsertTailList (LIST_ENTRY *ListHead, LIST_ENTRY *Entry)
{
    LIST_ENTRY *_ListHead;
    LIST_ENTRY *_BackLink;
    
    _ListHead           = ListHead;
    _BackLink           = _ListHead->BLink;
    Entry->FLink        = _ListHead;
    Entry->BLink        = _BackLink;
    _BackLink->FLink    = Entry;
    _ListHead->BLink    = Entry;
}

LIST_ENTRY Sprite::m_nodeHead;
bool init = InitializeListHead(&Sprite::m_nodeHead);


// MARK: create, init, dealloc
Sprite* Sprite::createWithTexture(Texture2D *texture)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && sprite->initWithTexture(texture))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::createWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && sprite->initWithTexture(texture, rect, rotated))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::create(const std::string& filename)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && sprite->initWithFile(filename))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::create(const PolygonInfo& info)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if(sprite && sprite->initWithPolygon(info))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::create(const std::string& filename, const Rect& rect)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && sprite->initWithFile(filename, rect))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

Sprite* Sprite::createWithSpriteFrameName(const std::string& spriteFrameName)
{
    SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
    
#if COCOS2D_DEBUG > 0
    char msg[256] = {0};
    sprintf(msg, "Invalid spriteFrameName: %s", spriteFrameName.c_str());
    CCASSERT(frame != nullptr, msg);
#endif
    
    return createWithSpriteFrame(frame);
}

Sprite* Sprite::create()
{
    Sprite *sprite = new (std::nothrow) Sprite();
    if (sprite && sprite->init())
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

bool Sprite::init(void)
{
    return initWithTexture(nullptr, Rect::ZERO );
}

bool Sprite::initWithTexture(Texture2D *texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");

    Rect rect = Rect::ZERO;
    rect.size = texture->getContentSize();

    return initWithTexture(texture, rect);
}

bool Sprite::initWithTexture(Texture2D *texture, const Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool Sprite::initWithFile(const std::string& filename)
{
    CCASSERT(filename.size()>0, "Invalid filename for sprite");

    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
    if (texture)
    {
        Rect rect = Rect::ZERO;
        rect.size = texture->getContentSize();
        return initWithTexture(texture, rect);
    }

    // don't release here.
    // when load texture failed, it's better to get a "transparent" sprite then a crashed program
    // this->release();
    return false;
}

bool Sprite::initWithFile(const std::string &filename, const Rect& rect)
{
    CCASSERT(filename.size()>0, "Invalid filename");

    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
    if (texture)
    {
        return initWithTexture(texture, rect);
    }

    // don't release here.
    // when load texture failed, it's better to get a "transparent" sprite then a crashed program
    // this->release();
    return false;
}

bool Sprite::initWithSpriteFrameName(const std::string& spriteFrameName)
{
    CCASSERT(spriteFrameName.size() > 0, "Invalid spriteFrameName");

    SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
    return initWithSpriteFrame(frame);
}

bool Sprite::initWithSpriteFrame(SpriteFrame *spriteFrame)
{
    CCASSERT(spriteFrame != nullptr, "");

    bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
    setSpriteFrame(spriteFrame);
//    CCLOG("liudi texture check, %d", spriteFrame->getTexture()->getAlphaName());

    return bRet;
}

bool Sprite::initWithPolygon(const cocos2d::PolygonInfo &info)
{
    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(info.filename);
    bool res = false;
    if(initWithTexture(texture))
    {
        _polyInfo = info;
        setContentSize(_polyInfo.rect.size/Director::getInstance()->getContentScaleFactor());
        res = true;
    }
    return res;
}

// designated initializer
bool Sprite::initWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
    bool result;
    if (Node::init())
    {
        _batchNode = nullptr;
        
        _recursiveDirty = false;
        setDirty(false);
        
        _opacityModifyRGB = true;
        
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
        
        _flippedX = _flippedY = false;
        
        // default transform anchor: center
        setAnchorPoint(Vec2(0.5f, 0.5f));
        
        // zwoptex default values
        _offsetPosition.setZero();

        // clean the Quad
        memset(&_quad, 0, sizeof(_quad));
        
        // Atlas: Color
        _quad.bl.colors = Color4B::WHITE;
        _quad.br.colors = Color4B::WHITE;
        _quad.tl.colors = Color4B::WHITE;
        _quad.tr.colors = Color4B::WHITE;
        
        // Modified by ChenFei 2014-12-26 V3.2 supports
        // setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
        if(texture && texture->getAlphaName())
        {
//            CCLOG("liudi use etc1");
            // shader state
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_ETC1_A_NO_MVP));
        }
        else
        {
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
        }
//        if(texture)
//            CCLOG("liudi test, %d, %d", texture->getAlphaName(), texture->getName());
//        else
//            CCLOG("liudi test, nullText");
        // update texture (calls updateBlendFunc)
        setTexture(texture);
        setTextureRect(rect, rotated, rect.size);
        
        _polyInfo.setQuad(&_quad);
        // by default use "Self Render".
        // if the sprite is added to a batchnode, then it will automatically switch to "batchnode Render"
        setBatchNode(nullptr);
        result = true;
    }
    else
    {
        result = false;
    }
    _recursiveDirty = true;
    setDirty(true);
    return result;
}

unsigned long Sprite::spriteCount = 0;

Sprite::Sprite(void)
: _batchNode(nullptr)
, _shouldBeHidden(false)
, _texture(nullptr)
, _spriteFrame(nullptr)
, _insideBounds(true)
,m_fScaleTexture(1)
,m_fScaleX0(1)
,m_fScaleY0(1)
,m_isSmallTexture(false)
{
#if CC_SPRITE_DEBUG_DRAW
    debugDraw(true);
#endif //CC_SPRITE_DEBUG_DRAW
    spriteCount++;
    if (Director::getInstance()->isDisplayStats()) {
        //lixu 20160217 count sprite for display
        
        Director::getInstance()->setSpriteCount(spriteCount);
    }
    
    // 加入列表
    InsertTailList(&m_nodeHead, &m_nodeEntry);
}

Sprite::~Sprite(void)
{
    // 移除列表
    RemoveEntryList(&m_nodeEntry);
    
    CC_SAFE_RELEASE(_spriteFrame);
    CC_SAFE_RELEASE(_texture);
    spriteCount--;
    if (Director::getInstance()->isDisplayStats()) {
        
        Director::getInstance()->setSpriteCount(spriteCount);
    }
}

/*
 * Texture methods
 */

/*
 * This array is the data of a white image with 2 by 2 dimension.
 * It's used for creating a default texture when sprite's texture is set to nullptr.
 * Supposing codes as follows:
 *
 *   auto sp = new (std::nothrow) Sprite();
 *   sp->init();  // Texture was set to nullptr, in order to make opacity and color to work correctly, we need to create a 2x2 white texture.
 *
 * The test is in "TestCpp/SpriteTest/Sprite without texture".
 */
static unsigned char cc_2x2_white_image[] = {
    // RGBA8888
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};

// MARK: texture
void Sprite::setTexture(const std::string &filename)
{
    Texture2D *texture = Director::getInstance()->getTextureCache()->addImage(filename);
    setTexture(texture);

    Rect rect = Rect::ZERO;
    if (texture)
        rect.size = texture->getContentSize();
    setTextureRect(rect);
}

void Sprite::setTexture(Texture2D *texture)
{
    // If batchnode, then texture id should be the same
    CCASSERT(! _batchNode || texture->getName() == _batchNode->getTexture()->getName(), "CCSprite: Batched sprites should use the same texture as the batchnode");
    // accept texture==nil as argument
    CCASSERT( !texture || dynamic_cast<Texture2D*>(texture), "setTexture expects a Texture2D. Invalid argument");

    if (texture == nullptr)
    {
        // Gets the texture by key firstly.
        texture = Director::getInstance()->getTextureCache()->getTextureForKey("cc_2x2_white_image");

        // If texture wasn't in cache, create it from RAW data.
        if (texture == nullptr)
        {
            Image* image = new (std::nothrow) Image();
            bool isOK = image->initWithRawData(cc_2x2_white_image, sizeof(cc_2x2_white_image), 2, 2, 8);
            CC_UNUSED_PARAM(isOK);
            CCASSERT(isOK, "The 2x2 empty texture was created unsuccessfully.");

            texture = Director::getInstance()->getTextureCache()->addImage(image, "cc_2x2_white_image");
//            texture->retain();
            CC_SAFE_RELEASE(image);
        }
    }

    if (!_batchNode && _texture != texture)
    {
        CC_SAFE_RETAIN(texture);
        CC_SAFE_RELEASE(_texture);
        _texture = texture;
        updateBlendFunc();
    }
    
//    bool isSmall = Texture2D::isSmallTexture(getTexture()->getFileName()); //modify by elex
//    if(isSmall==true){
//        m_fScaleTexture=2;
//        m_isSmallTexture=true;
//    }else{
        m_fScaleTexture=1;
        m_isSmallTexture=false;
//    }
    setScaleX(m_fScaleX0);
    setScaleY(m_fScaleY0);
}

Texture2D* Sprite::getTexture() const
{
    return _texture;
}

void Sprite::setTextureRect(const Rect& rect)
{
    setTextureRect(rect, false, rect.size);
}

void Sprite::setTextureRect(const Rect& rect, bool rotated, const Size& untrimmedSize)
{
    _rectRotated = rotated;

    setContentSize(untrimmedSize);
    setVertexRect(rect);
    setTextureCoords(rect);

    float relativeOffsetX = _unflippedOffsetPositionFromCenter.x;
    float relativeOffsetY = _unflippedOffsetPositionFromCenter.y;

    // issue #732
    if (_flippedX)
    {
        relativeOffsetX = -relativeOffsetX;
    }
    if (_flippedY)
    {
        relativeOffsetY = -relativeOffsetY;
    }

    _offsetPosition.x = relativeOffsetX + (_contentSize.width - _rect.size.width) / 2;
    _offsetPosition.y = relativeOffsetY + (_contentSize.height - _rect.size.height) / 2;

    // rendering using batch node
    if (_batchNode)
    {
        // update dirty_, don't update recursiveDirty_
        setDirty(true);
    }
    else
    {
        // self rendering
        
        // Atlas: Vertex
        float x1 = 0.0f + _offsetPosition.x;
        float y1 = 0.0f + _offsetPosition.y;
        float x2 = x1 + _rect.size.width;
        float y2 = y1 + _rect.size.height;

        // Don't update Z.
        _quad.bl.vertices.set(x1, y1, 0.0f);
        _quad.br.vertices.set(x2, y1, 0.0f);
        _quad.tl.vertices.set(x1, y2, 0.0f);
        _quad.tr.vertices.set(x2, y2, 0.0f);
    }
}

void Sprite::debugDraw(bool on)
{
    DrawNode* draw = getChildByName<DrawNode*>("debugDraw");
    if(on)
    {
        if(!draw)
        {
            draw = DrawNode::create();
            draw->setName("debugDraw");
            addChild(draw);
        }
        draw->setVisible(true);
        draw->clear();
        //draw lines
        auto last = _polyInfo.triangles.indexCount/3;
        auto _indices = _polyInfo.triangles.indices;
        auto _verts = _polyInfo.triangles.verts;
        for(unsigned int i = 0; i < last; i++)
        {
            //draw 3 lines
            Vec3 from =_verts[_indices[i*3]].vertices;
            Vec3 to = _verts[_indices[i*3+1]].vertices;
            draw->drawLine(Vec2(from.x, from.y), Vec2(to.x,to.y), Color4F::GREEN);
            
            from =_verts[_indices[i*3+1]].vertices;
            to = _verts[_indices[i*3+2]].vertices;
            draw->drawLine(Vec2(from.x, from.y), Vec2(to.x,to.y), Color4F::GREEN);
            
            from =_verts[_indices[i*3+2]].vertices;
            to = _verts[_indices[i*3]].vertices;
            draw->drawLine(Vec2(from.x, from.y), Vec2(to.x,to.y), Color4F::GREEN);
        }
    }
    else
    {
        if(draw)
            draw->setVisible(false);
    }
}


// override this method to generate "double scale" sprites
void Sprite::setVertexRect(const Rect& rect)
{
    _rect = rect;
}

void Sprite::setTextureCoords(const Rect& rectConst)
{
    // Modified by ChenFei 2015-02-26 for speeding up
    //rect = CC_RECT_POINTS_TO_PIXELS(rect);
    float scaleFactor = CC_CONTENT_SCALE_FACTOR();
    Rect& rect = const_cast<Rect&>(rectConst);
    rect.origin.x *= scaleFactor;
    rect.origin.y *= scaleFactor;
    rect.size.width *= scaleFactor;
    rect.size.height *= scaleFactor;

    Texture2D *tex = _batchNode ? _textureAtlas->getTexture() : _texture;
    if (! tex)
    {
        return;
    }

    float atlasWidth = (float)tex->getPixelsWide();
    float atlasHeight = (float)tex->getPixelsHigh();

    float left, right, top, bottom;

    if (_rectRotated)
    {
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
        left    = (2*rect.origin.x+1)/(2*atlasWidth);
        right   = left+(rect.size.height*2-2)/(2*atlasWidth);
        top     = (2*rect.origin.y+1)/(2*atlasHeight);
        bottom  = top+(rect.size.width*2-2)/(2*atlasHeight);
#else
        left    = rect.origin.x/atlasWidth;
        right   = (rect.origin.x+rect.size.height) / atlasWidth;
        top     = rect.origin.y/atlasHeight;
        bottom  = (rect.origin.y+rect.size.width) / atlasHeight;
#endif // CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

        if (_flippedX)
        {
            std::swap(top, bottom);
        }

        if (_flippedY)
        {
            std::swap(left, right);
        }

        _quad.bl.texCoords.u = left;
        _quad.bl.texCoords.v = top;
        _quad.br.texCoords.u = left;
        _quad.br.texCoords.v = bottom;
        _quad.tl.texCoords.u = right;
        _quad.tl.texCoords.v = top;
        _quad.tr.texCoords.u = right;
        _quad.tr.texCoords.v = bottom;
    }
    else
    {
#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
        left    = (2*rect.origin.x+1)/(2*atlasWidth);
        right    = left + (rect.size.width*2-2)/(2*atlasWidth);
        top        = (2*rect.origin.y+1)/(2*atlasHeight);
        bottom    = top + (rect.size.height*2-2)/(2*atlasHeight);
#else
        left    = rect.origin.x/atlasWidth;
        right    = (rect.origin.x + rect.size.width) / atlasWidth;
        top        = rect.origin.y/atlasHeight;
        bottom    = (rect.origin.y + rect.size.height) / atlasHeight;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

        if(_flippedX)
        {
            std::swap(left, right);
        }

        if(_flippedY)
        {
            std::swap(top, bottom);
        }

        _quad.bl.texCoords.u = left;
        _quad.bl.texCoords.v = bottom;
        _quad.br.texCoords.u = right;
        _quad.br.texCoords.v = bottom;
        _quad.tl.texCoords.u = left;
        _quad.tl.texCoords.v = top;
        _quad.tr.texCoords.u = right;
        _quad.tr.texCoords.v = top;
    }
}

// MARK: visit, draw, transform

void Sprite::updateTransform(void)
{
    CCASSERT(_batchNode, "updateTransform is only valid when Sprite is being rendered using an SpriteBatchNode");

    // recalculate matrix only if it is dirty
    if( isDirty() ) {

        // If it is not visible, or one of its ancestors is not visible, then do nothing:
        if( !_visible || ( _parent && _parent != _batchNode && static_cast<Sprite*>(_parent)->_shouldBeHidden) )
        {
            _quad.br.vertices.setZero();
            _quad.tl.vertices.setZero();
            _quad.tr.vertices.setZero();
            _quad.bl.vertices.setZero();
            _shouldBeHidden = true;
        }
        else
        {
            _shouldBeHidden = false;

            if( ! _parent || _parent == _batchNode )
            {
                _transformToBatch = getNodeToParentTransform();
            }
            else
            {
                CCASSERT( dynamic_cast<Sprite*>(_parent), "Logic error in Sprite. Parent must be a Sprite");
                const Mat4 &nodeToParent = getNodeToParentTransform();
                Mat4 &parentTransform = static_cast<Sprite*>(_parent)->_transformToBatch;
                _transformToBatch = parentTransform * nodeToParent;
            }

            //
            // calculate the Quad based on the Affine Matrix
            //

            Size &size = _rect.size;

            float x1 = _offsetPosition.x;
            float y1 = _offsetPosition.y;

            float x2 = x1 + size.width;
            float y2 = y1 + size.height;
            float x = _transformToBatch.m[12];
            float y = _transformToBatch.m[13];

            float cr = _transformToBatch.m[0];
            float sr = _transformToBatch.m[1];
            float cr2 = _transformToBatch.m[5];
            float sr2 = -_transformToBatch.m[4];
            float ax = x1 * cr - y1 * sr2 + x;
            float ay = x1 * sr + y1 * cr2 + y;

            float bx = x2 * cr - y1 * sr2 + x;
            float by = x2 * sr + y1 * cr2 + y;

            float cx = x2 * cr - y2 * sr2 + x;
            float cy = x2 * sr + y2 * cr2 + y;

            float dx = x1 * cr - y2 * sr2 + x;
            float dy = x1 * sr + y2 * cr2 + y;

            _quad.bl.vertices.set(RENDER_IN_SUBPIXEL(ax), RENDER_IN_SUBPIXEL(ay), _positionZ);
            _quad.br.vertices.set(RENDER_IN_SUBPIXEL(bx), RENDER_IN_SUBPIXEL(by), _positionZ);
            _quad.tl.vertices.set(RENDER_IN_SUBPIXEL(dx), RENDER_IN_SUBPIXEL(dy), _positionZ);
            _quad.tr.vertices.set(RENDER_IN_SUBPIXEL(cx), RENDER_IN_SUBPIXEL(cy), _positionZ);
        }

        // MARMALADE CHANGE: ADDED CHECK FOR nullptr, TO PERMIT SPRITES WITH NO BATCH NODE / TEXTURE ATLAS
        if (_textureAtlas)
        {
            _textureAtlas->updateQuad(&_quad, _atlasIndex);
        }

        _recursiveDirty = false;
        setDirty(false);
    }

    // MARMALADE CHANGED
    // recursively iterate over children
/*    if( _hasChildren )
    {
        // MARMALADE: CHANGED TO USE Node*
        // NOTE THAT WE HAVE ALSO DEFINED virtual Node::updateTransform()
        arrayMakeObjectsPerformSelector(_children, updateTransform, Sprite*);
    }*/
    Node::updateTransform();
}

// draw

void Sprite::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
#if CC_USE_CULLING
    // Don't do calculate the culling if the transform was not updated
    _insideBounds = (flags & FLAGS_TRANSFORM_DIRTY) ? renderer->checkVisibility(transform, _contentSize) : _insideBounds;

    if(_insideBounds)
#endif
    {
        _trianglesCommand.init(_globalZOrder, _texture->getName(), getGLProgramState(), _blendFunc, _polyInfo.triangles, transform, flags);  
        _trianglesCommand.setTextureID2(_texture->getAlphaName());
        renderer->addCommand(&_trianglesCommand);
    }
}

// MARK: visit, draw, transform

void Sprite::addChild(Node *child, int zOrder, int tag)
{
    CCASSERT(child != nullptr, "Argument must be non-nullptr");

    if (_batchNode)
    {
        Sprite* childSprite = dynamic_cast<Sprite*>(child);
        CCASSERT( childSprite, "CCSprite only supports Sprites as children when using SpriteBatchNode");
        CCASSERT(childSprite->getTexture()->getName() == _textureAtlas->getTexture()->getName(), "");
        //put it in descendants array of batch node
        _batchNode->appendChild(childSprite);

        if (!_reorderChildDirty)
        {
            setReorderChildDirtyRecursively();
        }
    }
    //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
    Node::addChild(child, zOrder, tag);
}

void Sprite::addChild(Node *child, int zOrder, const std::string &name)
{
    CCASSERT(child != nullptr, "Argument must be non-nullptr");
    
    if (_batchNode)
    {
        Sprite* childSprite = dynamic_cast<Sprite*>(child);
        CCASSERT( childSprite, "CCSprite only supports Sprites as children when using SpriteBatchNode");
        CCASSERT(childSprite->getTexture()->getName() == _textureAtlas->getTexture()->getName(), "");
        //put it in descendants array of batch node
        _batchNode->appendChild(childSprite);
        
        if (!_reorderChildDirty)
        {
            setReorderChildDirtyRecursively();
        }
    }
    //CCNode already sets isReorderChildDirty_ so this needs to be after batchNode check
    Node::addChild(child, zOrder, name);
}

void Sprite::reorderChild(Node *child, int zOrder)
{
    CCASSERT(child != nullptr, "child must be non null");
    CCASSERT(_children.contains(child), "child does not belong to this");

    if( _batchNode && ! _reorderChildDirty)
    {
        setReorderChildDirtyRecursively();
        _batchNode->reorderBatch(true);
    }

    Node::reorderChild(child, zOrder);
}

void Sprite::removeChild(Node *child, bool cleanup)
{
    if (_batchNode)
    {
        _batchNode->removeSpriteFromAtlas((Sprite*)(child));
    }

    Node::removeChild(child, cleanup);
}

void Sprite::removeAllChildrenWithCleanup(bool cleanup)
{
    if (_batchNode)
    {
        for(const auto &child : _children) {
            Sprite* sprite = dynamic_cast<Sprite*>(child);
            if (sprite)
            {
                _batchNode->removeSpriteFromAtlas(sprite);
            }
        }
    }

    Node::removeAllChildrenWithCleanup(cleanup);
}

void Sprite::sortAllChildren()
{
    if (_reorderChildDirty)
    {
        std::sort(std::begin(_children), std::end(_children), nodeComparisonLess);

        if ( _batchNode)
        {
            for(const auto &child : _children)
                child->sortAllChildren();
        }

        _reorderChildDirty = false;
    }
}

//
// Node property overloads
// used only when parent is SpriteBatchNode
//

void Sprite::setReorderChildDirtyRecursively(void)
{
    //only set parents flag the first time
    if ( ! _reorderChildDirty )
    {
        _reorderChildDirty = true;
        Node* node = static_cast<Node*>(_parent);
        while (node && node != _batchNode)
        {
            static_cast<Sprite*>(node)->setReorderChildDirtyRecursively();
            node=node->getParent();
        }
    }
}

void Sprite::setDirtyRecursively(bool bValue)
{
    _recursiveDirty = bValue;
    setDirty(bValue);

    for(const auto &child: _children) {
        Sprite* sp = dynamic_cast<Sprite*>(child);
        if (sp)
        {
            sp->setDirtyRecursively(true);
        }
    }
}

// FIXME: HACK: optimization
#define SET_DIRTY_RECURSIVELY() {                       \
                    if (! _recursiveDirty) {            \
                        _recursiveDirty = true;         \
                        setDirty(true);                 \
                        if (!_children.empty())         \
                            setDirtyRecursively(true);  \
                        }                               \
                    }

void Sprite::setPosition(const Vec2& pos)
{
    Node::setPosition(pos);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setPosition(float x, float y)
{
    Node::setPosition(x, y);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setRotation(float rotation)
{
    Node::setRotation(rotation);
    
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setRotationSkewX(float fRotationX)
{
    Node::setRotationSkewX(fRotationX);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setRotationSkewY(float fRotationY)
{
    Node::setRotationSkewY(fRotationY);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setSkewX(float sx)
{
    Node::setSkewX(sx);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setSkewY(float sy)
{
    Node::setSkewY(sy);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setScaleX(float scaleX)
{
    m_fScaleX0 = scaleX; //modify by elex
    Node::setScaleX(m_fScaleX0*m_fScaleTexture);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setScaleY(float scaleY)
{
    m_fScaleY0 = scaleY; //modify by elex
    Node::setScaleY(m_fScaleY0*m_fScaleTexture);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setScale(float fScale)
{
    m_fScaleX0 = m_fScaleY0 = fScale; //modify by elex
    Node::setScale(m_fScaleX0*m_fScaleTexture);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setScale(float scaleX, float scaleY)
{
    m_fScaleX0 = scaleX; //modify by elex
    m_fScaleY0 = scaleY;
    Node::setScale(m_fScaleX0*m_fScaleTexture,m_fScaleY0*m_fScaleTexture);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setPositionZ(float fVertexZ)
{
    Node::setPositionZ(fVertexZ);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setAnchorPoint(const Vec2& anchor)
{
    Node::setAnchorPoint(anchor);
    caclSamllParams(); //modify by elex
    SET_DIRTY_RECURSIVELY();
}

void Sprite::ignoreAnchorPointForPosition(bool value)
{
    CCASSERT(! _batchNode, "ignoreAnchorPointForPosition is invalid in Sprite");
    Node::ignoreAnchorPointForPosition(value);
}

void Sprite::setVisible(bool bVisible)
{
    Node::setVisible(bVisible);
    SET_DIRTY_RECURSIVELY();
}

void Sprite::setFlippedX(bool flippedX)
{
    if (_flippedX != flippedX)
    {
        _flippedX = flippedX;
        setTextureRect(_rect, _rectRotated, _contentSize);
    }
}

bool Sprite::isFlippedX(void) const
{
    return _flippedX;
}

void Sprite::setFlippedY(bool flippedY)
{
    if (_flippedY != flippedY)
    {
        _flippedY = flippedY;
        setTextureRect(_rect, _rectRotated, _contentSize);
    }
}

bool Sprite::isFlippedY(void) const
{
    return _flippedY;
}

//
// MARK: RGBA protocol
//

void Sprite::updateColor(void)
{
    Color4B color4( _displayedColor.r, _displayedColor.g, _displayedColor.b, _displayedOpacity );
    
    // special opacity for premultiplied textures
    if (_opacityModifyRGB)
    {
        color4.r *= _displayedOpacity/255.0f;
        color4.g *= _displayedOpacity/255.0f;
        color4.b *= _displayedOpacity/255.0f;
    }

    _quad.bl.colors = color4;
    _quad.br.colors = color4;
    _quad.tl.colors = color4;
    _quad.tr.colors = color4;

    // renders using batch node
    if (_batchNode)
    {
        if (_atlasIndex != INDEX_NOT_INITIALIZED)
        {
            _textureAtlas->updateQuad(&_quad, _atlasIndex);
        }
        else
        {
            // no need to set it recursively
            // update dirty_, don't update recursiveDirty_
            setDirty(true);
        }
    }

    // self render
    // do nothing
}

void Sprite::setOpacityModifyRGB(bool modify)
{
    if (_opacityModifyRGB != modify)
    {
        _opacityModifyRGB = modify;
        updateColor();
    }
}

bool Sprite::isOpacityModifyRGB(void) const
{
    return _opacityModifyRGB;
}

// MARK: Frames

void Sprite::setSpriteFrame(const std::string &spriteFrameName)
{
    SpriteFrameCache *cache = SpriteFrameCache::getInstance();
    SpriteFrame *spriteFrame = cache->getSpriteFrameByName(spriteFrameName);

    CCASSERT(spriteFrame, std::string("Invalid spriteFrameName :").append(spriteFrameName).c_str());

    setSpriteFrame(spriteFrame);
}

void Sprite::setSpriteFrame(SpriteFrame *spriteFrame)
{
    if (!spriteFrame) {  //modify by elex
        return;
    }
    // retain the sprite frame
    // do not removed by SpriteFrameCache::removeUnusedSpriteFrames
    if (_spriteFrame != spriteFrame)
    {
        CC_SAFE_RELEASE(_spriteFrame);
        _spriteFrame = spriteFrame;
        spriteFrame->retain();
    }
    _unflippedOffsetPositionFromCenter = spriteFrame->getOffset();

    Texture2D *texture = spriteFrame->getTexture();
    // update texture before updating texture rect
    if (texture != _texture)
    {
        if(texture && texture->getAlphaName()) //modify by elex
        {
//            CCLOG("liudi use etc1");
            // shader state
            setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_ETC1_A_NO_MVP));
        }
        else
        {
                        setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
        }
        setTexture(texture);
    }

    // update rect
    _rectRotated = spriteFrame->isRotated();
    setTextureRect(spriteFrame->getRect(), _rectRotated, spriteFrame->getOriginalSize());
}

void Sprite::setDisplayFrameWithAnimationName(const std::string& animationName, ssize_t frameIndex)
{
    CCASSERT(animationName.size()>0, "CCSprite#setDisplayFrameWithAnimationName. animationName must not be nullptr");

    Animation *a = AnimationCache::getInstance()->getAnimation(animationName);

    CCASSERT(a, "CCSprite#setDisplayFrameWithAnimationName: Frame not found");

    AnimationFrame* frame = a->getFrames().at(frameIndex);

    CCASSERT(frame, "CCSprite#setDisplayFrame. Invalid frame");

    setSpriteFrame(frame->getSpriteFrame());
}

bool Sprite::isFrameDisplayed(SpriteFrame *frame) const
{
    Rect r = frame->getRect();

    return (r.equals(_rect) &&
            frame->getTexture()->getName() == _texture->getName() &&
            frame->getOffset().equals(_unflippedOffsetPositionFromCenter));
}

SpriteFrame* Sprite::getSpriteFrame() const
{
    if(nullptr != this->_spriteFrame)
    {
        return this->_spriteFrame;
    }
    return SpriteFrame::createWithTexture(_texture,
                                           CC_RECT_POINTS_TO_PIXELS(_rect),
                                           _rectRotated,
                                           CC_POINT_POINTS_TO_PIXELS(_unflippedOffsetPositionFromCenter),
                                           CC_SIZE_POINTS_TO_PIXELS(_contentSize));
}

SpriteBatchNode* Sprite::getBatchNode() const
{
    return _batchNode;
}

void Sprite::setBatchNode(SpriteBatchNode *spriteBatchNode)
{
    _batchNode = spriteBatchNode; // weak reference

    // self render
    if( ! _batchNode ) {
        _atlasIndex = INDEX_NOT_INITIALIZED;
        setTextureAtlas(nullptr);
        _recursiveDirty = false;
        setDirty(false);

        float x1 = _offsetPosition.x;
        float y1 = _offsetPosition.y;
        float x2 = x1 + _rect.size.width;
        float y2 = y1 + _rect.size.height;
        _quad.bl.vertices.set( x1, y1, 0 );
        _quad.br.vertices.set(x2, y1, 0);
        _quad.tl.vertices.set(x1, y2, 0);
        _quad.tr.vertices.set(x2, y2, 0);

    } else {

        // using batch
        _transformToBatch = Mat4::IDENTITY;
        setTextureAtlas(_batchNode->getTextureAtlas()); // weak ref
    }
}

// MARK: Texture protocol

void Sprite::updateBlendFunc(void)
{
    CCASSERT(! _batchNode, "CCSprite: updateBlendFunc doesn't work when the sprite is rendered using a SpriteBatchNode");

    // it is possible to have an untextured sprite
    if (! _texture || ! _texture->hasPremultipliedAlpha())
    {
//        CCLOG("liudi no premulti");
        _blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
        setOpacityModifyRGB(false);
    }
    else
    {
//        CCLOG("liudi premulti");
        _blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
        setOpacityModifyRGB(true);
    }
}

std::string Sprite::getDescription() const
{
    int texture_id = -1;
    if( _batchNode )
        texture_id = _batchNode->getTextureAtlas()->getTexture()->getName();
    else
        texture_id = _texture->getName();
    return StringUtils::format("<Sprite | Tag = %d, TextureID = %d>", _tag, texture_id );
}

PolygonInfo Sprite::getPolygonInfo() const
{
    return _polyInfo;
}

void Sprite::setPolygonInfo(const PolygonInfo& info)
{
    _polyInfo = info;
}

Rect Sprite::getBoundingBox() const
{
    if(m_isSmallTexture==true){
        Rect rect = Rect(0, 0, _contentSize.width/m_fScaleTexture, _contentSize.height/m_fScaleTexture);
        
        float x = _position.x;
        float y = _position.y;
        Point anchorP = Point(rect.size.width * _anchorPoint.x, rect.size.height * _anchorPoint.y);
        if (_ignoreAnchorPointForPosition){
            x += anchorP.x;
            y += anchorP.y;
        }
        AffineTransform transform;
        // Rotation values
        // Change rotation code to handle X and Y
        // If we skew with the exact same value for both x and y then we're simply just rotating
        float cx = 1, sx = 0, cy = 1, sy = 0;
        if (_rotationX || _rotationY)
        {
            float radiansX = -CC_DEGREES_TO_RADIANS(_rotationX);
            float radiansY = -CC_DEGREES_TO_RADIANS(_rotationY);
            cx = cosf(radiansX);
            sx = sinf(radiansX);
            cy = cosf(radiansY);
            sy = sinf(radiansY);
        }
        
        bool needsSkewMatrix = ( _skewX || _skewY );
        
        
        // optimization:
        // inline anchor point calculation if skew is not needed
        // Adjusted transform calculation for rotational skew
        if (! needsSkewMatrix && !anchorP.equals(Point::ZERO))
        {
            x += cy * -anchorP.x * _scaleX + -sx * -anchorP.y * _scaleY;
            y += sy * -anchorP.x * _scaleX +  cx * -anchorP.y * _scaleY;
        }
        
        
        // Build Transform Matrix
        // Adjusted transform calculation for rotational skew
        transform = AffineTransformMake( cy * _scaleX,  sy * _scaleX,
                                          -sx * _scaleY, cx * _scaleY,
                                          x, y );
        
        // XXX: Try to inline skew
        // If skew is needed, apply skew and then anchor point
        if (needsSkewMatrix)
        {
            AffineTransform skewMatrix = AffineTransformMake(1.0f, tanf(CC_DEGREES_TO_RADIANS(_skewY)),
                                                                 tanf(CC_DEGREES_TO_RADIANS(_skewX)), 1.0f,
                                                                 0.0f, 0.0f );
            transform = AffineTransformConcat(skewMatrix, transform);
            
            // adjust anchor point
            if (!anchorP.equals(Point::ZERO))
            {
                transform = AffineTransformTranslate(transform, -anchorP.x, -anchorP.y);
            }
        }
        
        return RectApplyAffineTransform(rect, transform);
        
    }
    return Node::getBoundingBox();
}

void Sprite::setContentSize(const Size & size)
{
    if(m_isSmallTexture==false){
        Node::setContentSize(size);
    }else{
        if(!size.equals(_contentSize)){
            _contentSize = size * m_fScaleTexture;
            caclSamllParams();
            _transformDirty = _inverseDirty = true;
        }
    }
}
void Sprite::caclSamllParams(){
    if(m_isSmallTexture==true){
        Size size = _contentSize/m_fScaleTexture;
        Point tmpAnchorPoint = _anchorPoint - Point(0.5, 0.5);
        Size changeSize = _contentSize - size;
        _anchorPointInPoints = Point(_contentSize.width * _anchorPoint.x - changeSize.width * tmpAnchorPoint.x, _contentSize.height * _anchorPoint.y  - changeSize.height * tmpAnchorPoint.y);
    }
}
float Sprite::getScale() const
{
    CCAssert( m_fScaleX0 == m_fScaleY0, "CCNode#scale. ScaleX != ScaleY. Don't know which one to return");
    return m_fScaleX0;
}
float Sprite::getScaleX() const
{
    return m_fScaleX0;
}
float Sprite::getScaleY() const
{
    return m_fScaleY0;
}


// 添加程序，处理色相Hue修改
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

// 设置Hue的数值，为了不给之前的添加麻烦，没有新增变量，仅在这个函数里做处理
void Sprite::setHueDegree(GLfloat _hue)
{
    // 生成shader
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
    GLint m_hueLocation = glGetUniformLocation(this->getGLProgram()->getProgram(), "u_hue");
    // GLint m_alphaLocation = glGetUniformLocation(this->getGLProgram()->getProgram(), "u_alpha");
    this->updateColor();
    
    // 将hue从-180~180调整到-PI~PI
    GLfloat hue = _hue / 180 * M_PI;
    
    // 使用GLProgram
    this->getGLProgram()->use();
    GLfloat mat[3][3];
    memset(mat, 0, sizeof(GLfloat)*9);
    hueMatrix(mat, hue);
    premultiplyAlpha(mat, _displayedOpacity / 255.0f);
    glUniformMatrix3fv(m_hueLocation, 1, GL_FALSE, (GLfloat *)&mat);
}

NS_CC_END
