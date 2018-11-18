#ifndef EDITOR_H
#define EDITOR_H

#include <Project.h>

namespace sm {
namespace editor {

class Editor {
public:
    Editor();

    void editorOf(project::Project *project);
};

}
}


#endif //EDITOR_H
