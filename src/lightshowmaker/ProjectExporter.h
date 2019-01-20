#ifndef PROJECTEXPORTER_H
#define PROJECTEXPORTER_H

#include "Project.h"

namespace sm {

class ProjectExporter {
public:
    void exportIno(std::shared_ptr<sm::model::Project> proj, std::string filename);

private:
    std::shared_ptr<sm::model::Project> proj;

    void exportChunk(std::string filename, sm::model::arduino_number number) const;
    void exportCommons(std::string basic_string);
};

}

#endif //PROJECTEXPORTER_H
