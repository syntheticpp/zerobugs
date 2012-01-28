/** check.c
 *
 * @author Massimiliano Pagani
 * @version 1.0
 * @date 05/10/2006
 *
 * @notes
 * @history
 *
 */

typedef struct
{
    int a;
    void* b;
}
First;

First first;

typedef struct
{
    char const* name;
    char const* family;

    struct
    {
        const char* nickname;
    } x;
}
Second;

struct Middle
{
    First* ptr;
};

void
f( Second* s, int argc, char** argv )
{
    s->name = "Max";
    s->family = "Windsor";
    s->x.nickname = "Bigus";
}

int main( int argc, char** argv )
{
    Second second;
    f( &second, argc, argv );
    struct Middle middle;
    First* first = 0;
    middle.ptr = first;
    return 0;
}
