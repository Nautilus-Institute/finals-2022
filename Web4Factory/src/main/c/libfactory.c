#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "institute_nautilus_web4factory_LibFactory.h"

//#define eprintf(...) fprintf(stderr, ##__VA_ARGS__)
#define eprintf(...)

#define ENV ((*env))
#define EA JNIEnv* env

typedef uint32_t sptr;
#define P(c) ((void*)((uint64_t)(c)))
#define PT(t,c) ((t*)P(c))

struct _jobject {
    union {
        void* ptr;
        sptr sptr;
    } p; 
};

char g_error[0x100] = {0};

void set_err(char* c) {
    eprintf("In set_err for %p\n", c);
    if (*c == 0)
        return;
    strncpy(g_error, c, sizeof(g_error));
}


typedef struct jjarray {
    uint64_t header;
    uint32_t class;
    uint32_t length;
    char data[];
} jjarray;

typedef struct jstring {
    uint64_t header;
    uint32_t class;
    sptr unknown;
    jjarray* arr;
    jjarray* err;
} jjstring;

/*
void* get_data(arr) {
    return P(PT(jjarray,arr)->data);
}

char* get_str(sptr str) {
    return (char*)get_data(str->arr);
}
*/

void dump_mem(void* ptr) {
    uint32_t* p = ptr;
    for (size_t i=0; i<4; i++) {
        eprintf("%p: 0x%08x 0x%08x 0x%08x 0x%08x\n", ptr, p[0], p[1], p[2], p[3]);
        p += 4;
    }
}

JNIEXPORT jboolean JNICALL Java_institute_nautilus_web4factory_LibFactory_a (JNIEnv * env, jobject obj) {
    //puts("Loaded library");
    //puts("!!!!!!!!!!!!!!!!!!!! FORKING!!!");
    //int pid = fork();
    alarm(15);
    return 1;
}
/*
JNIEXPORT jint JNICALL Java_institute_nautilus_web4factory_LibFactory_b (JNIEnv * env, jobject self) {
    jclass clazz = (*env)->GetObjectClass(env, self);
    jfieldID read_fd_id = (*env)->GetFieldID(env, clazz, "read_fd", "I");
    if (read_fd_id == NULL) {
        return -1;
    }
    jfieldID write_fd_id = (*env)->GetFieldID(env, clazz, "write_fd", "I");
    if (write_fd_id == NULL) {
        return -1;
    }


    puts("Opening pipes");
    int parent_write[2];
    int parent_read[2];
    pipe(parent_write);
    pipe(parent_read);

    puts("!!!!!!!!!!!!!!!!!!!! FORKING!!!");
    int pid = fork();

    puts("Setting fields");
    if (pid == 0) {
        alarm(10);
        for (int i=3; i<256; i++) {
            if (i == parent_write[0] || i == parent_read[1])
                continue;
            close(i);
        in_fork = 1;
        (*env)->SetIntField(env, self, read_fd_id, parent_write[0]);
        (*env)->SetIntField(env, self, write_fd_id, parent_read[1]);
    } else {
        (*env)->SetIntField(env, self, write_fd_id, parent_write[1]);
        (*env)->SetIntField(env, self, read_fd_id, parent_read[0]);
        close(parent_read[1]);
        close(parent_write[0]);
    }
    return pid;
}
*/

jstring new_string(EA, char* s) {
    return ENV->NewStringUTF(env, s);
}

jobject createGrabLast(EA) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "grabLast", "()Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m);
}

jobject createRemoveEmptyLines(EA) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "removeEmptyLines", "()Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m);
}

jobject createNumberLines(EA) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "numberLines", "()Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m);
}

jobject createGrabNth(EA, jint ind) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "grabNth", "(I)Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m, ind);
}

jobject createJoinLines(EA, char* join) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "joinLines", "(Ljava/lang/String;)Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m, new_string(env, join));
}

jobject createPrefixLines(EA, char* pref) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryAwkWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetStaticMethodID(env, clazz, "prefixLines", "(Ljava/lang/String;)Ljava/lang/Object;");
    eprintf("jmethodID %p\n", m);
    eprintf("Calling CallStaticObjectMethod\n");
    return ENV->CallStaticObjectMethod(env, clazz, m, new_string(env, pref));
}

jobject createInput(EA) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryInputWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetMethodID(env, clazz, "<init>", "()V");
    eprintf("jmethodID %p\n", m);
    return ENV->NewObject(env, clazz, m);
}
jobject createConcat(EA) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactoryConcatWidget");
    eprintf("clazz is %p\n", clazz);
    jmethodID m = ENV->GetMethodID(env, clazz, "<init>", "()V");
    eprintf("jmethodID %p\n", m);
    return ENV->NewObject(env, clazz, m);
}

jobject createFactory(EA, char* name, jobject f) {
    jclass clazz = ENV->FindClass(env, "institute/nautilus/web4factory/web4factoryfactory/Web4FactoryFactory");
    jmethodID m = ENV->GetMethodID(env, clazz, "<init>", "(Ljava/lang/String;Ljava/lang/Object;)V");
    return ENV->NewObject(env, clazz, m, new_string(env, name), f);
}

int __attribute__((noinline)) is_space(char c) {
    return isspace(c);
}

typedef struct Tag {
    char* name;
    char* attr;
    struct Tag* child;
    struct Tag* next;
} Tag;

char* find_next_tag(char** data_p) {
    char* data = *data_p;
    eprintf("find_next_tag(`%s`)\n", data);
    long start = -1;
    long end = -1;
    for (size_t i=0; data[i]; i++) {
        char c = data[i];
        if (c == '[') {
            if (data[i+1] == '-')
                end = i-1;
            *data_p = &data[i+1];
            if (start != -1 && end != -1 && start <= end) {
                data[end + 1] = '\0';
                return &data[start];
            }
            return NULL;
        }
        int space = is_space(c);
        if (start == -1) {
            if (space) {
                continue;
            }
            start = i;
        }
        if (!space) {
            end = i;
        }
    }
    set_err(data);
    *data_p = NULL;
    return NULL;
}

Tag* str_tag(char* data) {
    Tag* t = calloc(1, sizeof(Tag));
    t->name = "string";
    t->attr = data;
    return t;
}

#define TAG_CLOSE (Tag*)(1)

void print_tag(Tag* t, int lvl) {
    for (int i=0; i<lvl; i++) {
        eprintf("-");
    }
    eprintf("[%s=%s]\n", t->name, t->attr?t->attr:"NULL");
    Tag* child = t->child;
    while (child) {
        print_tag(child, lvl+1);
        child = child->next;
    }
}

Tag* parse_tag(char** data_p, char** err) {
    Tag* t = calloc(1, sizeof(Tag));

    char* data = *data_p;

    eprintf("parse_tag(`%s`)\n", data);
    if (*data == '[')
        data++;
    if (*data == '-')
        data++;
    while (*data && is_space(*data))
        data++;

    int is_tag_close = 0;
    int closed = 0;
    int ended = 0;
    for (size_t i=0; data[i]; i++) {
        char c = data[i];
        if (c == '/') {
            if (i > 0) {
                t->name = data;
                data[i] = '\0';
            } else if (t->name == NULL) {
                // Parent tag close
                is_tag_close = 1;
            }
            closed = 1;
            continue;
        }
        if ((c == ']' || c == '=') && t->name == NULL) {
            if (i == 0) {
                *err = "Expected block name";
                free(t);
                return NULL;
            }
            t->name = data;
            data[i] = '\0';
        }
        if (c == ']') {
            data[i] = '\0';

            ended = 1;
            *data_p = &data[i+1];
            break;
        }
        if (c == '=') {
            if (t->attr != NULL) {
                *err = "Only one value is allowed in each block";
                free(t);
                return NULL;
            }
            data[i] = '\0';
            t->attr = &data[i+1];
        }
    }
    if (!ended) {
        free(t);
        *err = "Expected closing `]` on block";
        set_err(*err);
        return NULL;
    }

    if (is_tag_close) {
        free(t);
        return TAG_CLOSE;
    }
    if (closed) {
        // No children
        eprintf("Finished tag:\n");
        print_tag(t, 0);
        return t;
    }
    Tag* child_tag = NULL;
    for (size_t i = 0; !closed; i++) {
        Tag* at = NULL;

        if (i % 2 == 0) {
            char* mdata = *data_p;
            char* arg = find_next_tag(data_p);
            if (*data_p == NULL) {
                *err = mdata;
                set_err(*err);
                return NULL;
            }
            if (arg == NULL) {
                continue;
            }
            at = str_tag(arg);
        } else {
            at = parse_tag(data_p, err);
            if (*err != NULL)
                return NULL;
            if (at == TAG_CLOSE) {
                closed = 1;
                break;
            }
        }

        if (child_tag == NULL) {
            t->child = at;
        } else {
            child_tag->next = at;
        }
        child_tag = at;
    }

    eprintf("Finished tag:\n");
    print_tag(t, 0);
    return t;
}


Tag* parse(char* data_in, char** err) {
    char* name = NULL;

    char* data = data_in;
    eprintf("Parsing `%s`\n", data);

    find_next_tag(&data);
    if (data == NULL)  {
        *err = data_in;
        set_err(*err);
        return NULL;
    }
    Tag* tag = parse_tag(&data, err);
    if (*err != NULL) {
        return NULL;
    }
    eprintf("Final parsed tag is:\n");
    print_tag(tag, 0);
    return tag;
}

int add_child(EA, jobject p, jobject c) {
    jclass clazz = ENV->GetObjectClass(env, p);
    if (clazz == NULL)
        return 0;
    jmethodID m = ENV->GetMethodID(env, clazz, "addChild", "(Ljava/lang/Object;)V");
    if (m == NULL)
        return 0;
    ENV->CallVoidMethod(env, p, m, c);
    return 1;
}

jobject build_widget(EA, Tag* t, char** err) {
    jobject widget = NULL;

    eprintf("Creating widget for tag:\n");
    print_tag(t, 0);

    if (!strcasecmp(t->name, "GrabLast"))
        widget = createGrabLast(env);
    else if (!strcasecmp(t->name, "RemoveEmptyLines"))
        widget = createRemoveEmptyLines(env);
    else if (!strcasecmp(t->name, "NumberLines"))
        widget = createNumberLines(env);
    else if (!strcasecmp(t->name, "Concat"))
        widget = createConcat(env);
    else if (!strcasecmp(t->name, "Input"))
        widget = createInput(env);
    else {
        // Blocks with attributes
        if (!strcasecmp(t->name, "String")) {
            if (t->attr == NULL) goto missing_attr;
            widget = new_string(env, t->attr);
        } else if (!strcasecmp(t->name, "GrabNth")) {
            if (t->attr == NULL) goto missing_attr;
            widget = createGrabNth(env, atoi(t->attr));
        } else if (!strcasecmp(t->name, "JoinLines")) {
            if (t->attr == NULL) goto missing_attr;
            widget = createJoinLines(env, t->attr);
        } else if (!strcasecmp(t->name, "PrefixLines")) {
            if (t->attr == NULL) goto missing_attr;
            widget = createPrefixLines(env, t->attr);
        }
    }

    if (widget == NULL) {
        *err = "Encounted unknown block %s";
        set_err(*err);
        //sprintf("Encounted unknown tag %s", t->name);
        return NULL;
missing_attr:
        *err = "Expected attribute for block %s";
        set_err(*err);
        return NULL;

    }

    Tag* ctag = t->child;

    while (ctag) {
        jobject child = build_widget(env, ctag, err);
        if (*err != NULL)
            return NULL;
        if (child == NULL || !add_child(env, widget, child)) {
            *err = "Internal error while building factory";
            set_err(*err);
            return NULL;
        }
        ctag = ctag->next;
    }

    return widget;
}

jobject build_factory(EA, Tag* t, char** err) {
    if (t->name == NULL || strcasecmp(t->name, "Web4Factory")) {
        *err = "Expected toplevel block to be [Web4Factory]";
        set_err(*err);
        return NULL;
    }
    jobject factory = NULL;
    jobject widget = NULL;
    Tag* tag = t->child;
    if (tag != NULL) {
        eprintf("Going to build widget\n");
        widget = build_widget(env, tag, err);
        if (*err != NULL) {
            return NULL;
        }
    }

    eprintf("Creating factory\n");
    factory = createFactory(env, t->attr?t->attr:"undefined", widget);
    return factory;
}


void copy_err(jjstring* str) {
    eprintf("In copy_err(%p)\n", str);
    jjarray* a = str->err;
    eprintf("byte[] is %p\n", a);
    char* data = (char*)P(a->data);
    eprintf("Copying %p to %p\n", g_error, data);
    char* err = g_error;
    while (*err) {
        *data = *err;
        err ++;
        data ++;
    }
}

// Write to other side
JNIEXPORT jobject JNICALL Java_institute_nautilus_web4factory_LibFactory_b (JNIEnv * env, jobject self, jobject arg1) {
    /*
    jclass clazz = (*env)->GetObjectClass(env, self);
    jfieldID write_fd_id = (*env)->GetFieldID(env, clazz, "write_fd", "I");
    if (write_fd_id == NULL) {
        return NULL;
    }
    int write_fd = (*env)->GetIntField(env, self, write_fd_id);
    */

    int pid = getpid();
    jjstring* j = PT(jjstring, arg1->p.sptr);
    //jjarray* a = PT(jjarray, j->arr);
    //void* data = P(a->data);
    jjarray* a = j->arr;
    void* data = P(a->data);
    eprintf("Got data `%s`\n", data);

    char* err = NULL;
    Tag* tags = parse(a->data, &err);
    if (err != NULL) {
        eprintf("Error Parsing: `%s`\n", err);
        copy_err(j);
        // TODO return error object
        return NULL;
    }

    eprintf("Going to build factory for %p\n", tags);
    jobject res = build_factory(env, tags, &err);
    if (err != NULL) {
        eprintf("Error Parsing: `%s`\n", err);
        copy_err(j);
        // TODO return error object
        return NULL;
    }

    //jstring s = new_string(env, "hello world\nfoo bar\n");
    //res = createGrabLast(env, s);
    //res = createFactory(env, "foobar", res);



    //char* data = get_str(arg1->p.sptr);
    //__asm("int3");
    return res;
}

