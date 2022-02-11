#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>

/* XPM */
static char *icon_xpm[] = {
    "32 23 3 1",
    "     c #FFFFFF",
    ".    c #000000",
    "+    c #FFFF00",
    "                                ",
    "            ........            ",
    "          ..++++++++..          ",
    "         .++++++++++++.         ",
    "        .++++++++++++++.        ",
    "       .++++++++++++++++.       ",
    "      .++++++++++++++++++.      ",
    "      .+++....++++....+++.      ",
    "     .++++.. .++++.. .++++.     ",
    "     .++++....++++....++++.     ",
    "     .++++++++++++++++++++.     ",
    "     .++++++++++++++++++++.     ",
    "     .+++++++++..+++++++++.     ",
    "     .+++++++++..+++++++++.     ",
    "     .++++++++++++++++++++.     ",
    "      .++++++++++++++++++.      ",
    "      .++...++++++++...++.      ",
    "       .++............++.       ",
    "        .++..........++.        ",
    "         .+++......+++.         ",
    "          ..++++++++..          ",
    "            ........            ",
    "                                ",
};

struct Texture
{
    uint         w, h, id;
    SDL_Surface *surface;

    Texture ()
    {
        w = h = id = 0;
        surface    = nullptr;
    }

    Texture (char **xpm)
    {
        init (xpm);
    }

    void init (char **xpm)
    {
        surface = IMG_ReadXPMFromArray (xpm);

        assert (surface != NULL);

        surface = SDL_ConvertSurfaceFormat (surface, SDL_PIXELFORMAT_RGBA32, 0);

        int mode            = GL_RGB;
        int internal_format = GL_SRGB_ALPHA;

        if (surface->format->BytesPerPixel == 4)
            mode = GL_RGBA;

        glGenTextures (1, &id);
        glBindTexture (GL_TEXTURE_2D, id);

        glTexImage2D (GL_TEXTURE_2D, 0, internal_format, surface->w, surface->h,
                      0, mode, GL_UNSIGNED_BYTE, surface->pixels);

        glGenerateMipmap (GL_TEXTURE_2D);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        w = surface->w;
        h = surface->h;
    }
};

typedef GLuint uint;
typedef GLint  sint;

template <class K, class V> class kv
{
    K key;
    V val;

    inline bool operator< (kv<K, V> b)
    {
        return (val < b.val) ? true : false;
    }

    inline bool operator> (kv<K, V> b)
    {
        return b < (*this);
    }

    inline bool operator== (kv<K, V> b)
    {
        return (b.key == key && b.val == val) ? true : false;
    }
};

template <class T> struct List
{
    struct Node
    {
        T     data;
        Node *next, *prev;

        Node (T data)
        {
            this->data = data;
            next = prev = nullptr;
        }
    };

    Node *first, *last;

    List ()
    {
        first = last = nullptr;
    }

    void push (T data)
    {
        Node curr = new Node (data);

        if (!first)
        {
            first = curr;
        }
        else
        {
            last->next = curr;
            curr->prev = last;
        }

        last = curr;
    }
};

template <class T> struct Array
{
    T     *data;
    size_t length, size;

    Array ()
    {
        data   = nullptr;
        length = size = 0;
    }

    template <class... Args> Array (T val, Args... args)
    {
        data   = nullptr;
        length = size = 0;

        push (val, args...);
    }

    template <class... Args> void push (T val, Args... args)
    {
        push (val);
        push (args...);
    }

    void resize ()
    {
        if (size == 0)
        {
            data = new T[1];
            size = 1;
        }
        else if (length >= size)
        {
            T *new_data = new T[size *= 2];

            for (size_t i = 0; i < length; i++)
                new_data[i] = data[i];

            delete data;

            data = new_data;
        }
    }

    void push (T value)
    {
        resize ();

        data[length++] = value;
    }

    T &operator[] (size_t index)
    {
        return (index >= length) ? data[length - 1] : data[index];
    }
};

template <class T> struct Tree
{
    struct Node
    {
        T     data;
        int   weight;
        Node *left, *right;

        Node (T data, Node *left = nullptr, Node *right = nullptr)
        {
            this->data  = data;
            this->left  = left;
            this->right = right;
        }
    };

    Node *root;

    Tree ()
    {
        root = nullptr;
    }

    template <class... Args> Tree (T val, Args... args)
    {
        root = nullptr;
        push (val, args...);
    }

    template <class... Args> void push (T data, Args... args)
    {
        root = push (data, root);
        push (args...);
    }

    Node *push (T data, Node *leaf)
    {
        if (!leaf)
            return new Node (data);

        if (data < leaf->data)
            leaf->left = push (data, leaf->left);
        else
            leaf->right = push (data, leaf->right);

        return leaf;
    }

    void push (T data)
    {
        root = push (data, root);
    }

    void push (Array<T> data)
    {
        for (size_t i = 0; i < data.length; i++)
            push (data[i]);
    }
};

void print (Tree<int>::Node *node)
{
    if (!node)
        return;

    printf ("%d\n", node->data);

    print (node->left);
    print (node->right);
}

struct string : public Array<char>
{
    char *c_str_data;

    string ()
    {
        c_str_data = nullptr;
    }

    string (const char *str)
    {
        for (int i = 0; str[i] != '\0'; i++)
            push (str[i]);
    }

    char *c_str ()
    {
        if (c_str_data != nullptr)
            free (c_str_data);

        c_str_data = new char[length + 1];

        size_t i = 0;

        for (; i < length; i++)
            c_str_data[i] = this->data[i];

        c_str_data[i] = '\0';

        return c_str_data;
    }

    inline bool operator< (string b)
    {
        assert (b.length > 0 && length > 0);

        for (size_t i = 0; i < length && i < b.length; i++)
        {
            if (data[i] < b[i])
                return true;
            else if (data[i] > b[i])
                return false;
        }

        return false;
    }

    inline bool operator> (string b)
    {
        return b < (*this);
    }

    inline bool operator== (string b)
    {
        assert (b.length > 0 && length > 0);

        if (b.length != length)
            return false;

        for (size_t i = 0; i < length; i++)
        {
            if (data[i] != b[i])
                return false;
        }

        return true;
    }
};

struct Shader
{
    void compile (const char *code, GLuint type)
    {
    }

    void init (const char *vs_code, const char *fs_code)
    {
    }
};

namespace shader
{
    uint compile (const char *code, uint type)
    {
        sint shader = glCreateShader (type), is_compiled = 0;

        glShaderSource (shader, 1, &code, 0);
        glCompileShader (shader);
        glGetShaderiv (shader, GL_COMPILE_STATUS, &is_compiled);

        if (is_compiled == GL_FALSE)
        {
            char info_log[512];
            sint max_length = 0;

            glGetShaderiv (shader, GL_INFO_LOG_LENGTH, &max_length);
            glGetShaderInfoLog (shader, max_length, &max_length, &info_log[0]);

            glDeleteShader (shader);

            assert (is_compiled != GL_FALSE);
        }

        return shader;
    }
}

struct Vec2
{
    float  data[2];
    float &x = data[0], &y = data[1];

    float &operator[] (int index)
    {
        switch (index)
        {
            case 0: return x;
            case 1: return y;
            default: return x;
        }
    }
};

struct Vec4
{
    float  data[4];
    float &x = data[0], &y = data[1], &z = data[2], &w = data[3];

    Vec4 ()
    {
    }

    Vec4 (float x, float y, float z, float w)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    float &operator[] (uint index)
    {
        assert (index < 4);

        return data[index];
    }
};

struct Mat4
{
    float data[4][4];

    Mat4 (Vec4 a, Vec4 b, Vec4 c, Vec4 d)
    {
        data[0][0] = a.x;
        data[0][1] = a.y;
        data[0][2] = a.z;
        data[0][3] = a.w;

        data[1][0] = b.x;
        data[1][1] = b.y;
        data[1][2] = b.z;
        data[1][3] = b.y;

        data[2][0] = c.x;
        data[2][1] = c.y;
        data[2][2] = c.z;
        data[2][3] = c.y;

        data[3][0] = d.x;
        data[3][1] = d.w;
        data[3][2] = d.z;
        data[3][3] = d.w;
    }

    float *operator[] (uint i)
    {
        assert (i < 4);

        return data[i];
    }
};

Mat4 identity ()
{
    return {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    };
}

int main (int argc, char **argv)
{
    SDL_Init (SDL_INIT_EVERYTHING);
    TTF_Init ();

    TTF_Font *font = TTF_OpenFont (
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf", 12);

    if (!font)
        printf ("font: %s\n", SDL_GetError ());

    SDL_Window *window
        = SDL_CreateWindow ("shipcade", 0, 0, 1280, 720, SDL_WINDOW_OPENGL);

    bool      run = true;
    SDL_Event event;

    if (!window)
    {
        printf ("SDL_Window error: %s\n", SDL_GetError ());
        return 0;
    }

    SDL_GL_CreateContext (window);
    SDL_GL_SetSwapInterval (0);

    glewInit ();

    Tree<int> tree = {
        3, 2, 5, 4, 1,
    };

    print (tree.root);

    Texture spritesheet (icon_xpm);

    Vec2 lul;

    lul.x = 5;

    lul[0] = 5;
    lul[0] = 1;

    while (run)
    {
        while (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
                case SDL_QUIT: run = false; break;
            }
        }

        glClearColor (0, 0, 0, 1);
        glClear (GL_CLEAR_BUFFER);

        SDL_GL_SwapWindow (window);
    }
}
