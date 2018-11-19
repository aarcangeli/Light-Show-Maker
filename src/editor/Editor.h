#ifndef EDITOR_H
#define EDITOR_H

#include <Project.h>

namespace sm {
namespace editor {

class Editor {
public:
    Editor();

    void editorOf(project::Project *project);

private:
    bool firstShow = true;
    void editorComponent(project::Canvas &canvas);
    void editorComponent(const char *label, time_unit &time);

    void parseTime(const char *buf, time_unit &time);
    inline void skipWhitespace(std::string &buf);
};

}
}


#endif //EDITOR_H
