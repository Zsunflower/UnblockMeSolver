#include "SlideBlock.h"

int SlideBlock::numberOfBlock = 1;

Vec2 SlideBlock::ORIGIN = Vec2::ZERO;

SlideBlock::SlideBlock(std::vector<Coor> &coor, TYPE type)
{
    _lenght = coor.size();
    _type = type;
    _id = numberOfBlock++;
    bodyCoor = coor;
    for(auto c : coor)
    {
        SlideBlock::_matrix[c.x][c.y] = _id;
    }
}

SlideBlock *SlideBlock::create(std::vector<Coor> &coor, TYPE type)
{
    SlideBlock *ret = new(std::nothrow) SlideBlock(coor, type);
    if(ret && ret->init())
    {
        ret->autorelease();
        return ret;
    }
    else
    {
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
}

size_t SlideBlock::hashCurrentMatrix()
{
    int i, j;
    size_t result = 0;
    std::hash<int> hasher;
    for(i = 0; i < 6; ++i)
    {
        for(j = 0; j < 6; ++j)
        {
            result ^= hasher(_matrix[i][j]) + 0x9e3779b9 + (result << 6) + (result >> 2);
        }
    }
    return result;
}

void SlideBlock::resetMatrix()
{
    int i, j;
    for(i = 0; i < 6; ++i)
    {
        for(j = 0; j < 6; ++j)
        {
            _matrix[i][j] = 0;
        }
    }
    numberOfBlock = 1;
}

int SlideBlock::getSlideBlockID() const
{
    return _id;
}

TYPE SlideBlock::type() const
{
    return _type;
}

int SlideBlock::getFinalMoveDistance()
{
    return 5 - bodyCoor.back().x;
}

void SlideBlock::setFinalSlideBlock()
{
    if(_type == TYPE::VERTICAL)
    {
        if(bodyCoor.size() == 2)
            spriteBody->setSpriteFrame(
                    SpriteFrameCache::getInstance()->getSpriteFrameByName("verf2.png"));
        else
            spriteBody->setSpriteFrame(
                    SpriteFrameCache::getInstance()->getSpriteFrameByName("verf3.png"));
    }
    else
    {
        if(bodyCoor.size() == 2)
            spriteBody->setSpriteFrame(
                    SpriteFrameCache::getInstance()->getSpriteFrameByName("hozf2.png"));
        else
            spriteBody->setSpriteFrame(
                    SpriteFrameCache::getInstance()->getSpriteFrameByName("hozf3.png"));
    }
}

bool SlideBlock::init()
{
    if(_type == TYPE::VERTICAL)
    {
        if(bodyCoor.size() == 2)
            spriteBody = Sprite::createWithSpriteFrameName("ver2.png");
        else
            spriteBody = Sprite::createWithSpriteFrameName("ver3.png");
    }
    else
    {
        if(bodyCoor.size() == 2)
            spriteBody = Sprite::createWithSpriteFrameName("hoz2.png");
        else
            spriteBody = Sprite::createWithSpriteFrameName("hoz3.png");
    }
    auto numberLabel = Label::createWithTTF(to_string(_id), "fonts/Marker Felt.ttf", 40);
    numberLabel->setPosition(spriteBody->getContentSize() / 2);
    spriteBody->addChild(numberLabel);

    spriteBody->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
    spriteBody->setPosition(ORIGIN + Vec2(bodyCoor.front().x * 84, bodyCoor.front().y * 84));
    this->addChild(spriteBody);
    return true;
}

void SlideBlock::releaseBlock()
{
    --numberOfBlock;
    for(auto c : bodyCoor)
    {
        SlideBlock::_matrix[c.x][c.y] = 0;
    }
}

void SlideBlock::ownBlock()
{
    ++numberOfBlock;
    for(auto c : bodyCoor)
    {
        SlideBlock::_matrix[c.x][c.y] = _id;
    }
}

void SlideBlock::ownBlockParse()
{
    for(auto c : bodyCoor)
    {
        SlideBlock::_matrix[c.x][c.y] = _id;
    }
}

void SlideBlock::popReverse()
{
    trace.pop_back();
}

bool SlideBlock::isValidMove(int distance)
{
    int d = 0;
    Coor coor, offset;
    if(distance == 0)
    {
        return true;
    }
    if(distance > 0)
    {
        if(_type == TYPE::VERTICAL)
        {
            offset.x = 0;
            offset.y = 1;
        }
        else
        {
            offset.x = 1;
            offset.y = 0;
        }
        coor = bodyCoor.back();
    }
    else
    {
        if(_type == TYPE::VERTICAL)
        {
            offset.x = 0;
            offset.y = -1;
        }
        else
        {
            offset.x = -1;
            offset.y = 0;
        }
        coor = bodyCoor.front();
    }
    coor += offset;
    while(isValidPosition(coor) && isFreePosition(coor))
    {
        ++d;
        coor += offset;
        if(d == ABS(distance))
            return true;
    }
    return false;
}

void SlideBlock::moveBy(int distance)
{
    Coor offset;
    if(_type == TYPE::VERTICAL)
    {
        offset = Coor(0, distance);
    }
    else
    {
        offset = Coor(distance, 0);
    }
    for(auto c : bodyCoor)
    {
        SlideBlock::_matrix[c.x][c.y] = 0;
    }
    for(auto &c : bodyCoor)
    {
        c += offset;
        SlideBlock::_matrix[c.x][c.y] = _id;
    }
    trace.push_back(distance);
}

void SlideBlock::refreshPosition(float delay, std::function<void()> cbOnFinish)
{
    if(trace.empty())
        return;

    int distance = trace.back();
    Vec2 moveDistance;
    if(_type == TYPE::VERTICAL)
        moveDistance = Vec2(0.0f, distance * 84);
    else
        moveDistance = Vec2(distance * 84, 0.0f);
    this->runAction(
            Sequence::create(MoveBy::create(delay, moveDistance), CallFunc::create(cbOnFinish),
                             nullptr));
}

void SlideBlock::reverseMove()
{
    if(trace.empty())
        return;

    int v = trace.back();
    trace.pop_back();
    moveBy(-v);
}

bool SlideBlock::possibleReachTo(Coor coor)
{
    if(!isFreePosition(coor))
        return false;
    if(_type == TYPE::VERTICAL)
    {
        if(bodyCoor.front().x != coor.x)
            return false;
        return coor.y < bodyCoor.front().y ? isValidMove(coor.y - bodyCoor.front().y) : isValidMove(
                coor.y - bodyCoor.back().y);
    }
    else
    {
        if(bodyCoor.front().y != coor.y)
            return false;
        return coor.x < bodyCoor.front().x ? isValidMove(coor.x - bodyCoor.front().x) : isValidMove(
                coor.x - bodyCoor.back().x);
    }
}

void SlideBlock::getAllMove(std::vector<int> &move)
{
    int i;
    for(i = _lenght - 6; i <= 6 - _lenght; ++i)
    {
        if(i != 0 && isValidMove(i))
            move.push_back(i);
    }
}

void SlideBlock::getFreeCoorIfMove(std::vector<Coor> &coor, int distance)
{
    int i, n = ABS(distance) <= _lenght ? ABS(distance) : _lenght;
    if(distance > 0)
        for(i = 0; i < n; ++i)
        {
            coor.push_back(bodyCoor[i]);
        }
    else
        for(i = _lenght - n; i < _lenght; ++i)
        {
            coor.push_back(bodyCoor[i]);
        }
}

void SlideBlock::setBodyCoor(std::vector<Coor> &_bodyCoor)
{
    trace.clear();
    bodyCoor = _bodyCoor;
    for(auto c : bodyCoor)
    {
        SlideBlock::_matrix[c.x][c.y] = _id;
    }
    this->setPosition(Vec2::ZERO);
}

void SlideBlock::logdebug()
{
    int i;
    for(i = 0; i < _lenght; ++i)
    {
        log("(%d, %d)", bodyCoor[i].x, bodyCoor[i].y);
    }
}

SlideBlock::~SlideBlock()
{
}
