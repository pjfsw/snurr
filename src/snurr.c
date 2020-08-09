#define GL_GLEXT_PROTOTYPES
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_opengl_glext.h>

typedef struct {
    SDL_Window *window;
    SDL_GLContext context;
} Screen;

#define SCRW 640
#define SCRH 480

void cleanup(Screen **screenPtr) {
    Screen *screen = *screenPtr;
    if (screen != NULL) {
        if (screen->context != NULL) {
            SDL_GL_DeleteContext(screen->context);
            screen->context = NULL;
        }
        if (screen->window != NULL) {
            SDL_DestroyWindow(screen->window);
            screen->window = NULL;
        }
        free(*screenPtr);
        *screenPtr = NULL;
        SDL_Quit();
    }
}

Screen *init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL2: %s\n", SDL_GetError());
        return NULL;
    }
    Screen *screen = calloc(1, sizeof(Screen));

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    screen->window = SDL_CreateWindow(
            "Snurr",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCRW, SCRH,
            SDL_WINDOW_OPENGL
            );
    if (screen->window == NULL) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        cleanup(&screen);
        return NULL;
    }
    screen->context = SDL_GL_CreateContext(screen->window);
    if (screen->context == NULL) {
        fprintf(stderr, "Could not create GL context: %s\n", SDL_GetError());
        cleanup(&screen);
        return NULL;
    }

    return screen;
}

void print_gl_info() {
    int r, g, b, major,minor;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &r);
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &g);
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &b);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    printf("GL Version %d.%d, Red size: %d, Green size: %d, Blue size: %d\n", major, minor, r, g, b);

}

int create_shader_program(const char *vs_src, const char *fs_src) {
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, fs);
    glAttachShader(program, vs);
    glLinkProgram(program);
    return program;
}

void run(Screen *screen) {
    float points[] = {
         0.0f, 0.5f,  0.0f,
         0.5f, -0.5f,  0.0f,
         -0.5f,-0.5f, 0.0f
    };
    int vertexCount = sizeof(points)/(3 * sizeof(float));

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const char* vertex_shader1 =
        "#version 400\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp, 1.0);"
        "}";

    const char* fragment_shader1 =
        "#version 400\n"
        "out vec4 frag_colour;"
        "void main() {"
        "  frag_colour = vec4(0.9, 0.0, 0.5, 0.7);"
        "}";

    const char* vertex_shader2 =
        "#version 400\n"
        "in vec3 vp;"
        "void main() {"
        "  gl_Position = vec4(vp.x+0.5,vp.y,vp.z, 1.0);"
        "}";


    const char* fragment_shader2 =
        "#version 400\n"
        "out vec4 frag_colour;"
        "void main() {"
        "  frag_colour = vec4(0.0, 0.6, 0.9, 0.7);"
        "}";


    GLuint sp1 = create_shader_program(vertex_shader1, fragment_shader1);
    GLuint sp2 = create_shader_program(vertex_shader2, fragment_shader2);

    bool quit = false;
    while( !quit ) {
        /* Poll for events */
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            default:
                break;
            }
        }
        glViewport(0, 0, SCRW, SCRH);
        glClearColor(0.2,0.2,0.2,0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(sp1);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glUseProgram(sp2);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        SDL_GL_SwapWindow(screen->window);
        SDL_Delay(2);
    }
}

int main(int argc, char** argv) {
    Screen *screen = init();
    if (screen != NULL) {
        print_gl_info();
        run(screen);
        cleanup(&screen);
        return 0;
    } else {
        return 1;
    }
}
