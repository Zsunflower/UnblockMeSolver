#ifndef SLIDE_BLOCK_H
#define SLIDE_BLOCK_H

#include "cocos2d.h"
#include "Ultils.h"
#include <stack>

USING_NS_CC;

#define ABS(X) ((X) > 0 ? (X) : -(X))

enum class TYPE
{
    VERTICAL, HORIZONTAL
};

enum class GO
{
    BACKWARD, FORWARD
};


struct Coor
{
    int x, y;

    Coor()
    {
        x = y = 0;
    }
    Coor(int _x, int _y)
    {
        x = _x;
        y = _y;
    }
    bool operator==(const Coor &coor)
    {
        return (x == coor.x) && (y == coor.y);
    }
    Coor operator+(const Coor &coor)
    {
        return Coor(x + coor.x, y + coor.y);
    }
    Coor &operator+=(const Coor &coor)
    {
        x += coor.x;
        y += coor.y;
        return *this;
    }
};

class SlideBlock : public Layer
{
private:
    int _lenght;

    TYPE _type;

    std::vector<Coor> bodyCoor;

    Sprite *spriteBody;

    std::vector<int> trace;

    int _id;

    static int numberOfBlock;

    static int _matrix[6][6];

public:
    static Vec2 ORIGIN;

    SlideBlock(std::vector<Coor> &coor, TYPE type);
    static SlideBlock *create(std::vector<Coor> &coor, TYPE type);
    static size_t hashCurrentMatrix();
    static void resetMatrix();
    int getSlideBlockID() const;
    TYPE type() const;
    int getFinalMoveDistance();
    void setFinalSlideBlock();
    virtual bool init();
    void releaseBlock();
    void ownBlock();
    void ownBlockParse();
    void popReverse();
    bool possibleReachTo(Coor coor);
    void getAllMove(std::vector<int> &move);
    void getFreeCoorIfMove(std::vector<Coor> &coor, int distance);
    void setBodyCoor(std::vector<Coor> &_bodyCoor);
    bool static inline isValidPosition(const Coor &coor)
    {
        return (coor.x >= 0 && coor.x < 6) && (coor.y >= 0 && coor.y < 6);
    }
    bool static inline isValidPosition(int x, int y)
    {
        return (x >= 0 && x < 6) && (y >= 0 && y < 6);
    }
    bool static inline isFreePosition(const Coor &coor)
    {
        return _matrix[coor.x][coor.y] == 0;
    }
    bool static inline isFreePosition(int x, int y)
    {
        return _matrix[x][y] == 0;
    }
    bool isValidMove(int distance);
    void moveBy(int distance);
    void reverseMove();
    void refreshPosition(float delay, std::function<void()> cbOnFinish);
    void logdebug();
    ~SlideBlock();
};

#endif // !SLIDE_BLOCK_H
