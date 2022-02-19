#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>

/* XPM */
static char* icon_xpm[] = {
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

#include "font.xpm"

template <int size, class VecN> struct Vec
{
    float data[size];

    void init ()
    {
        for (int i = 0; i < size; i++) data[i] = 0;
    }

    float& operator[] (int index)
    {
        assert (index < size);

        return data[index];
    }

    VecN& equals (const VecN& value)
    {
        for (int i = 0; i < size; i++) data[i] = value.data[i];

        return (VecN&)*this;
    }

    VecN operator+ (VecN value)
    {
        VecN result;

        for (int i = 0; i < size; i++) result[i] = data[i] + value[i];

        return result;
    }

    VecN operator- (VecN value)
    {
        VecN result;

        for (int i = 0; i < size; i++) result[i] = data[i] - value[i];

        return result;
    }

    VecN operator* (float value)
    {
        VecN result;

        for (int i = 0; i < size; i++) result[i] = data[i] * value;

        return result;
    }
};

struct Vec2 : public Vec<2, Vec2>
{
    float &x = data[0], &y = data[1];

    Vec2 () { init (); }

    Vec2 (float x, float y)
    {
        data[0] = x;
        data[1] = y;
    }

    Vec2& operator= (const Vec2& val) { return equals (val); }
};

struct Vec3 : public Vec<3, Vec3>
{
    float &x = data[0], &y = data[1], &z = data[2];

    Vec3 () { init (); }

    Vec3 (float x, float y, float z)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }
};

struct Vec4 : public Vec<4, Vec4>
{
    float &x = data[0], &y = data[1], &z = data[2], &w = data[3];

    Vec4 () { init (); }

    Vec4 (float x, float y, float z, float w)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }
};

struct Mat4
{
    float data[4][4];

    Mat4 ()
    {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++) data[i][j] = 0;
    }

    Mat4 (Vec4 a, Vec4 b, Vec4 c, Vec4 d)
    {
        for (int i = 0; i < 4; i++)
        {
            data[0][i] = a[i];
            data[1][i] = b[i];
            data[2][i] = c[i];
            data[3][i] = d[i];
        }
    }

    float* operator[] (uint i)
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

inline Mat4 ortho (float W, float H)
{
    float r = W, t = 0;
    float l = 0, b = H;
    float f = 1, n = -1;

    Mat4 matrix = identity ();

    matrix[0][0] = 2.f / (r - l);
    matrix[0][3] = -(r + l) / (r - l);

    matrix[1][1] = 2.f / (t - b);
    matrix[1][3] = -(t + b) / (t - b);

    matrix[2][2] = -2.f / (f - n);
    matrix[2][3] = -(f + n) / (f - n);

    return matrix;
}

inline Mat4 mul (Mat4 m1, Mat4 m2)
{
    Mat4 result;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            float val = 0;

            for (int k = 0; k < 4; k++) { val += (m1[i][k] * m2[k][j]); }

            result[i][j] = val;
        }
    }

    return result;
}

inline void translate (Mat4& matrix, Vec2 pos)
{
    Mat4 trans_matrix = identity ();

    // {1, 0, 0, pos.x}
    // {0, 1, 0, pos.y}
    // {0, 0, 1, 0    }
    // {0, 0, 0, 1    }

    trans_matrix[0][3] = pos.x;
    trans_matrix[1][3] = pos.y;

    matrix = mul (matrix, trans_matrix);
}

inline void scale (Mat4& matrix, Vec2 size)
{
    Mat4 scale_matrix = identity ();

    // {size.x, 0,      0, 0}
    // {0,      size.y, 0, 0}
    // {0,      0,      1, 0}
    // {0,      0,      0, 1}

    scale_matrix[0][0] = size.x;
    scale_matrix[1][1] = size.y;

    matrix = mul (matrix, scale_matrix);
}

inline void rotate (Mat4& matrix, float angle)
{
    // {size.x, 0,      0, 0}
    // {0,      size.y, 0, 0}
    // {0,      0,      1, 0}
    // {0,      0,      0, 1}

    angle = angle * (3.1415f / 180.f);

    Mat4 rotation_matrix = identity ();

    rotation_matrix[0][0] = cos (angle);
    rotation_matrix[0][1] = (-sin (angle));

    rotation_matrix[1][0] = sin (angle);
    rotation_matrix[1][1] = cos (angle);

    matrix = mul (matrix, rotation_matrix);
}

inline Mat4 get_model (Vec2 pos, Vec2 size, float angle = 0)
{
    Mat4 matrix = identity ();

    translate (matrix, pos);
    translate (matrix, size * (0.5f));

    rotate (matrix, angle);
    translate (matrix, size * (-0.5f));

    scale (matrix, size);

    return matrix;
}

struct Texture
{
    uint         w, h, id;
    SDL_Surface* surface;

    Texture ()
    {
        w = h = id = 0;
        surface    = nullptr;
    }

    Texture (char** xpm) { init (xpm); }

    void init (char** xpm)
    {
        surface = IMG_ReadXPMFromArray (xpm);

        assert (surface != NULL);

        surface = SDL_ConvertSurfaceFormat (surface, SDL_PIXELFORMAT_RGBA32, 0);

        int mode            = GL_RGB;
        int internal_format = GL_SRGB_ALPHA;

        if (surface->format->BytesPerPixel == 4) mode = GL_RGBA;

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

template <class K, class V> struct kv
{
    K key;
    V val;

    inline bool operator< (kv<K, V> b) { return (val < b.val) ? true : false; }

    inline bool operator> (kv<K, V> b) { return b < (*this); }

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

    List () { first = last = nullptr; }

    void push (T data)
    {
        Node curr = new Node (data);

        if (!first) { first = curr; }
        else {
            last->next = curr;
            curr->prev = last;
        }

        last = curr;
    }
};

template <class T> struct Array
{
    T*     data;
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
        else if (length >= size) {
            T* new_data = new T[size *= 2];

            for (size_t i = 0; i < length; i++) new_data[i] = data[i];

            delete data;

            data = new_data;
        }
    }

    void push (T value)
    {
        resize ();

        data[length++] = value;
    }

    T& operator[] (size_t index)
    {
        return (index >= length) ? data[length - 1] : data[index];
    }

    void clean ()
    {
        if (data) delete data;

        data = nullptr;

        length = size = 0;
    }
};

template <class T> struct Tree
{
    struct Node
    {
        T     data;
        int   weight;
        Node *left, *right;

        Node (T data, Node* left = nullptr, Node* right = nullptr)
        {
            this->data  = data;
            this->left  = left;
            this->right = right;
        }
    };

    Node* root;

    Tree () { root = nullptr; }

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

    Node* push (T data, Node* leaf)
    {
        if (!leaf) return new Node (data);

        if (data < leaf->data) leaf->left = push (data, leaf->left);
        else leaf->right = push (data, leaf->right);

        return leaf;
    }

    void push (T data) { root = push (data, root); }

    void push (Array<T> data)
    {
        for (size_t i = 0; i < data.length; i++) push (data[i]);
    }

    int height (Node* node, int i)
    {
        if (!node) return i - 1;

        if (height (node->left, i + 1) > height (node->right, i + 1))
            return height (node->left, i + 1);
        else return height (node->right, i + 1);
    }

    int height () { return height (root, 1); }
};

void print (Tree<int>::Node* node)
{
    if (!node) return;

    printf ("%d\n", node->data);

    print (node->left);
    print (node->right);
}

struct string : public Array<char>
{
    char* c_str_data;

    string () { c_str_data = nullptr; }

    string (const char* str)
    {
        c_str_data = nullptr;

        for (int i = 0; str[i] != '\0'; i++) push (str[i]);
    }

    string (size_t size)
    {
        c_str_data = nullptr;

        this->size = size;
        data       = new char[size];
    }

    void clean ()
    {
        if (c_str_data) delete c_str_data;
        if (data) delete data;
    }

    char* c_str ()
    {
        if (c_str_data != nullptr) free (c_str_data);

        c_str_data = new char[length + 1];

        size_t i = 0;

        for (; i < length; i++) c_str_data[i] = this->data[i];

        c_str_data[i] = '\0';

        return c_str_data;
    }

    inline bool operator< (string b)
    {
        assert (b.length > 0 && length > 0);

        for (size_t i = 0; i < length && i < b.length; i++)
        {
            if (data[i] < b[i]) return true;
            else if (data[i] > b[i]) return false;
        }

        return false;
    }

    inline bool operator> (string b) { return b < (*this); }

    inline bool operator== (string b)
    {
        assert (b.length > 0 && length > 0);

        if (b.length != length) return false;

        for (size_t i = 0; i < length; i++)
        {
            if (data[i] != b[i]) return false;
        }

        return true;
    }
};

string read_file (const char* filename)
{
    FILE* fp = fopen (filename, "r");

    assert (fp != nullptr);

    fseek (fp, 0, SEEK_END);
    size_t size = ftell (fp);

    fseek (fp, 0, SEEK_SET);

    string str (size);

    char c = '0';

    while ((c = fgetc (fp)) != EOF) str.push (c);

    fclose (fp);

    return str;
}

const float W = 1280.f;
const float H = 720.f;

struct Shader
{
    uint vao, vbo;
    sint id, vertex, fragment;

    Shader () { id = vertex = fragment = vbo = vao = 0; }

    Shader (const char* vs_file, const char* fs_file)
    {
        init (vs_file, fs_file);
    }

    void set (const char* name, sint val)
    {
        glUniform1i (glGetUniformLocation (id, name), val);
    }

    void set (const char* name, float val)
    {
        glUniform1f (glGetUniformLocation (id, name), val);
    }

    void set (const char* name, Vec3 val)
    {
        glUniform3fv (glGetUniformLocation (id, name), 1, val.data);
    }

    void set (const char* name, Vec4 val)
    {
        glUniform4fv (glGetUniformLocation (id, name), 1, val.data);
    }

    void set (const char* name, Vec2 val)
    {
        glUniform2fv (glGetUniformLocation (id, name), 1, val.data);
    }

    void set (const char* name, Mat4 val)
    {
        glUniformMatrix4fv (glGetUniformLocation (id, name), 1, GL_TRUE,
                            val[0]);
    }

    sint compile (string code, uint type)
    {
        char* cstr   = code.c_str ();
        sint  shader = glCreateShader (type), is_compiled = 0;

        glShaderSource (shader, 1, &cstr, 0);
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

        code.clean ();

        return shader;
    }

    void init (const char* vs_file, const char* fs_file)
    {
        vertex   = compile (read_file (vs_file), GL_VERTEX_SHADER);
        fragment = compile (read_file (fs_file), GL_FRAGMENT_SHADER);

        id = glCreateProgram ();

        glAttachShader (id, vertex);
        glAttachShader (id, fragment);

        glLinkProgram (id);

        sint is_linked;
        glGetProgramiv (id, GL_LINK_STATUS, &is_linked);

        assert (is_linked != false);

        glUseProgram (id);
        set ("u_color", { 1, 1, 0 });
        set ("u_projection", ortho (W, H));

        set ("u_image", 0);
        set ("u_offset", { 0, 0, .1, .1 });
        set ("u_alpha", 1.f);

        init_buffers ();
    }

    void init_buffers ()
    {
        const float points[] = {
            0.0f, 1.0f,    //
            1.0f, 0.0f,    //
            0.0f, 0.0f,    //

            0.0f, 1.0f,    //
            1.0f, 1.0f,    //
            1.0f, 0.0f,    //
        };

        glGenVertexArrays (1, &vao);
        glGenBuffers (1, &vbo);

        glBindVertexArray (vao);
        glBindBuffer (GL_ARRAY_BUFFER, vbo);

        glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray (0);

        glBufferData (GL_ARRAY_BUFFER, sizeof (points), points, GL_STATIC_DRAW);
    }

    void use ()
    {
        glBindVertexArray (vao);
        glUseProgram (id);
    }
};

Array<kv<float, Vec2> > nodes;

void graphical_nodes (Tree<float>::Node* node, Vec2 size, float w = 0,
                      float h = 0, int offset = 0)
{
    if (!node) return;

    nodes.push ({ node->data, { w * size.x, h * size.y } });

    graphical_nodes (node->left, size, w - offset, h + 1.2, offset - 1.f);
    graphical_nodes (node->right, size, w + offset, h + 1.2, offset - 1.f);
}

int main (int argc, char** argv)
{
    SDL_Init (SDL_INIT_EVERYTHING);
    TTF_Init ();

    TTF_Font* font = TTF_OpenFont (
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf", 12);

    if (!font) printf ("font: %s\n", SDL_GetError ());

    SDL_Window* window
        = SDL_CreateWindow ("shipcade", 0, 0, W, H, SDL_WINDOW_OPENGL);

    bool      run = true;
    SDL_Event event;

    if (!window)
    {
        printf ("SDL_Window error: %s\n", SDL_GetError ());
        return 0;
    }

    SDL_GL_CreateContext (window);

    glewInit ();

    SDL_GL_SetSwapInterval (0);

    Tree<float> tree = { 5, 3, 2, 4, 7, 6, 8, 15, 10, 9, 11, 16, 4.5, 5.5 };

    Texture spritesheet (font_xpm);

    Shader shader ("vertex.glsl", "fragment.glsl");

    Vec2 height = {
        W / ((tree.height () * 8) + tree.height ()),
        H / (tree.height () * 8),
    };

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (run)
    {
        while (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
                case SDL_QUIT: run = false; break;
            }
        }

        glClearColor (0.f, 0.f, 0.f, 1.f);
        glClear (GL_COLOR_BUFFER_BIT);

        graphical_nodes (tree.root, height, tree.height () * 4, 0,
                         tree.height ());

        shader.use ();
        glBindTexture (GL_TEXTURE0, spritesheet.id);

        for (size_t i = 0; i < nodes.length; i++)
        {
            char p[10];

            // TODO: .1 after calculating per subtree spacing
            sprintf (p, "%.0f", nodes[i].key);

            for (size_t j = 0; j < strlen (p); j++)
            {
                Vec4 offset = { 0, .4, .1, .1 };

                if (p[j] == '.')
                {
                    // TODO: first work with in-between space
                    //  offset.x = .6;
                    //  offset.y = .2;
                    continue;
                }
                else offset.x = (p[j] - 48) / 10.f;

                shader.set ("u_offset", offset);
                shader.set ("u_model", get_model (nodes[i].val, height, 0));

                glDrawArrays (GL_TRIANGLES, 0, 6);

                nodes[i].val.x += 16;
            }
        }

        SDL_GL_SwapWindow (window);

        nodes.length = 0;
    }
}
