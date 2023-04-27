typedef enum
{
    RED = 41,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    WHITE,
    BLACK = 0   ,
}Color ;

typedef enum
{
    EMPTY = -1,
    I,
    J,
    L,
    O,
    S,
    T,
    Z
}ShapeId;

typedef struct 
{
    Color color;
    ShapeId shape;
}Block;

