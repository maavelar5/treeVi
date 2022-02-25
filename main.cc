#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cassert>

#include "font.xpm"

enum OPERATORS
{
    ADD,
    SUB,
    MUL,
    DIV,
};

namespace vec
{
    template <class T> T operators (T a, T b, const int OP)
    {
        T result;

        for (size_t i = 0; i < a.axis; i++)
        {
            switch (OP)
            {
                case ADD: result[i] = a[i] + b[i]; break;
                case SUB: result[i] = a[i] - b[i]; break;
                case MUL: result[i] = a[i] * b[i]; break;
                case DIV: result[i] = a[i] / b[i]; break;
            }
        }

        return result;
    }

    template <class T> T operators (T a, float scalar, const int OP)
    {
        T result;

        for (size_t i = 0; i < a.axis; i++)
        {
            switch (OP)
            {
                case ADD: result[i] = a[i] + scalar; break;
                case SUB: result[i] = a[i] - scalar; break;
                case MUL: result[i] = a[i] * scalar; break;
                case DIV: result[i] = a[i] / scalar; break;
            }
        }

        return result;
    }

    template <class T> float length (T t)
    {
        float result = 0;

        for (size_t i = 0; i < t.axis; i++) result += (t[i] * t[i]);

        return fsqrt (result);
    }

    template <class T> T normalize (T v) { return v / v.length (); }

    template <class T> float& at (T& v, size_t index)
    {
        assert (index < v.axis);
        return v.data[index];
    }

    template <class T> void init (T& v)
    {
        for (size_t i = 0; i < v.axis; i++) v[i] = 0;
    }

    template <class T> T& copy (T& a, const T& b)
    {
        for (size_t i = 0; i < a.axis; i++) a[i] = b.data[i];

        return a;
    }
}

#define VEC_CONSTRUCTORS(type)     \
    type () { vec::init (*this); } \
    type (const type& val) { vec::copy (*this, val); }

#define VEC_FUNCTIONS(type)                               \
    type  normalize () { return vec::normalize (*this); } \
    float length () { return vec::length (*this); }

#define VEC_OPERATORS(type)                                               \
    type   operator+ (type v) { return vec::operators (*this, v, ADD); }  \
    type   operator- (type v) { return vec::operators (*this, v, SUB); }  \
    type   operator* (type v) { return vec::operators (*this, v, MUL); }  \
    type   operator/ (type v) { return vec::operators (*this, v, DIV); }  \
    type   operator+ (float v) { return vec::operators (*this, v, ADD); } \
    type   operator- (float v) { return vec::operators (*this, v, SUB); } \
    type   operator* (float v) { return vec::operators (*this, v, MUL); } \
    type   operator/ (float v) { return vec::operators (*this, v, DIV); } \
    type&  operator= (const type& val) { return vec::copy (*this, val); } \
    float& operator[] (size_t index) { return vec::at (*this, index); }

#define VEC(size)                \
    float  data[size];           \
    size_t axis = size;          \
    VEC_CONSTRUCTORS (Vec##size) \
    VEC_FUNCTIONS (Vec##size)    \
    VEC_OPERATORS (Vec##size)

struct Vec2
{
    float &x = data[0], &y = data[1];

    Vec2 (float x, float y)
    {
        data[0] = x;
        data[1] = y;
    }

    VEC (2);

    float angle (Vec2 b)
    {
        // dot = x1*x2 + y1*y2      # dot product between [x1, y1]
        // and [x2, y2] det = x1*y2 - y1*x2      # determinant angle
        // = atan2(det, dot)

        Vec2  c  = ((*this) - b).normalize ();
        float PI = 3.141592, degrees = 180.f, angle = 0.f;

        angle = atan2 (c.y, c.x) * degrees / PI;

        return angle;
    }
};

struct Vec3
{
    float &x = data[0], &y = data[1], &z = data[2];

    Vec3 (float x, float y, float z)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    VEC (3);
};

struct Vec4
{
    float &x = data[0], &y = data[1], &z = data[2], &w = data[3];

    Vec4 (float x, float y, float z, float w)
    {
        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;
    }

    VEC (4);
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

inline Mat4 get_model (Vec2 pos, Vec2 size, float angle = 0, bool center = true)
{
    Mat4 matrix = identity ();

    translate (matrix, pos);
    translate (matrix, size * (0.5f));

    rotate (matrix, angle);

    if (center) translate (matrix, size * (-0.5f));

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
    Texture (SDL_Surface* surface) { init (surface); }

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

    void init (SDL_Surface* surface)
    {
        assert (surface != NULL);

        this->surface
            = SDL_ConvertSurfaceFormat (surface, SDL_PIXELFORMAT_RGBA32, 0);

        int mode            = GL_RGB;
        int internal_format = GL_SRGB_ALPHA;

        if (this->surface->format->BytesPerPixel == 4) mode = GL_RGBA;

        glGenTextures (1, &id);
        glBindTexture (GL_TEXTURE_2D, id);

        glTexImage2D (GL_TEXTURE_2D, 0, internal_format, this->surface->w,
                      this->surface->h, 0, mode, GL_UNSIGNED_BYTE,
                      this->surface->pixels);

        glGenerateMipmap (GL_TEXTURE_2D);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        w = this->surface->w;
        h = this->surface->h;
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

    static int height (Node* node, int i)
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

struct Node_Reference
{
    float           key;
    Vec2            val;
    Node_Reference* parent;
};

Array<Node_Reference> nodes;

void graphical_nodes (Tree<float>::Node* node, float x = 0, float y = 0,
                      Node_Reference* parent = nullptr)
{
    if (!node) return;

    float height = Tree<float>::height (node, 1) * 2;

    Vec2 pos = { x + height, y };

    nodes.push ({ node->data, (pos * 16.f), parent });

    Node_Reference* nr = &nodes[nodes.length - 1];

    graphical_nodes (node->left, x, y + 2, nr);
    graphical_nodes (node->right, pos.x, y + 2, nr);
}

Array<kv<char, Texture> > charset;

TTF_Font* font = nullptr;

Texture get_char (char c)
{
    for (size_t i = 0; i < charset.length; i++)
    {
        printf ("%c\n", charset[i].key);
        if (charset[i].key == c) return charset[i].val;
    }

    const char text[2] = { c, '\0' };

    SDL_Surface* surface = TTF_RenderText_Solid (font, text, { 255, 255, 200 });

    Texture t (surface);

    charset.push ({ c, t });

    return t;
}

int main (int argc, char** argv)
{
    SDL_Init (SDL_INIT_EVERYTHING);
    TTF_Init ();

    SDL_Window* window
        = SDL_CreateWindow ("shipcade", 0, 0, W, H, SDL_WINDOW_OPENGL);

    assert (window != nullptr);

    bool      run = true;
    SDL_Event event;

    if (!window)
    {
        printf ("SDL_Window error: %s\n", SDL_GetError ());
        assert (window != nullptr);
        return 0;
    }

    SDL_GL_CreateContext (window);

    glewInit ();

    SDL_GL_SetSwapInterval (0);

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    font = TTF_OpenFont (
        "/usr/share/fonts/liberation/LiberationMono-Regular.ttf", 14);

    if (!font) printf ("font: %s\n", SDL_GetError ());

    Tree<float> tree = { 5, 3, 2, 4, 7, 6, 8, 15, 10, 9, 11, 16, 15, 13 };

    Texture spritesheet (font_xpm);

    Shader shader ("vertex.glsl", "fragment.glsl");

    Vec2 height = { 16, 16 };

    int fw, fh;

    TTF_SizeText (font, "a", &fw, &fh);

    Vec2 mouse;

    while (run)
    {
        while (SDL_PollEvent (&event))
        {
            switch (event.type)
            {
                case SDL_QUIT: run = false; break;
                case SDL_MOUSEMOTION:
                    int x, y;

                    SDL_GetMouseState (&x, &y);

                    mouse.x = x;
                    mouse.y = y;
                    break;
            }
        }

        glClearColor (0.f, 0.f, 0.f, 1.f);
        glClear (GL_COLOR_BUFFER_BIT);

        shader.use ();
        glBindTexture (GL_TEXTURE0, spritesheet.id);

        graphical_nodes (tree.root);

        for (size_t i = 0; i < nodes.length; i++)
        {
            char p[20];

            snprintf (p, 20, "%.0f", nodes[i].key);

            Vec2 pos = nodes[i].val;
            shader.set ("u_type", 0);

            if (nodes[i].parent)
            {
                Vec2 node = nodes[i].val;

                char parent_string[20];

                snprintf (parent_string, 20, "%.0f", nodes[i].key);

                Vec2 parent = nodes[i].parent->val;
                Vec2 diff   = (parent - node) / 2.f;
                Vec2 size   = { (parent - node).x, 2 };
                Vec2 line   = { node.x + ((strlen (p) * 16.f) / 2.f),
                              node.y + diff.y + 8 };

                float angle = parent.angle (line);

                shader.set ("u_model", get_model (line, size, angle));
                glDrawArrays (GL_TRIANGLES, 0, 6);
            }

#if 1
            shader.set ("u_type", 1);

            for (size_t j = 0; j < strlen (p); j++)
            {
                Vec4 offset = { 0, .4, .1, .1 };

                if (p[j] == '.')
                {
                    offset.x = .6;
                    offset.y = .2;
                }
                else offset.x = (p[j] - 48) / 10.f;

                shader.set ("u_offset", offset);
                shader.set ("u_model", get_model (pos, height, 0));
                glDrawArrays (GL_TRIANGLES, 0, 6);

                pos.x += 16;
            }
#endif

#if 0
            shader.set ("u_type", 2);

            for (size_t j = 0; j < strlen (p); j++)
            {
                Texture t = get_char (p[j]);

                glBindTexture (GL_TEXTURE_2D, t.id);

                shader.set ("u_model", get_model (pos, { (float)fw, 16 }, 0));
                glDrawArrays (GL_TRIANGLES, 0, 6);

                pos.x += 16;
            }
#endif
        }
        nodes.length = 0;

        SDL_GL_SwapWindow (window);
    }

    SDL_Quit ();
}
