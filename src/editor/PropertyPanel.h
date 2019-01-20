#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include "Selection.h"
#include "functional"

namespace sm {
namespace editor {

class PropertyPanel {
    using KeyRef = std::shared_ptr<model::KeyPoint>;

public:
    void showProperties(SelectionManager &manager);
    void showPropertiesOf(std::shared_ptr<model::Layer> &layer);
    void showPropertiesOf(std::shared_ptr<model::Decoration> &layer);
    void showPropertiesOf(std::shared_ptr<model::KeyPoint> &keypoint);
    void showPropertiesOf(model::Fade &keypoint, const char *name, time_unit max);

    void multiKeypointEditor(SelectionManager &manager);

private:
    template<typename T>
    void bulkEditor(const char *name, SelectionManager &manager,
                    std::function<T &(KeyRef)> getter,
                    std::function<T(KeyRef)> maxGetter);

    model::Fade fade;

    // wrapper of Imgui with beginCommand/endCommand
    bool showSlider(const char *name, int &item, int min, int max);
    bool showSlider(const char *name, float &item, float min, float max, const char* format = "%.3f");
    bool showInputInt(const char *name, int &item);
    bool showInputFloat(const char *name, float &item);
    bool showCombo(const char *name, int *item, const char **items, int items_count);
    bool showInputText(const char *name, std::string &value);
    bool showColorEdit3(const char *name, uint32_t &value);
    bool somethingChanged = false;
    void startChanging(const char *name = "Change property");
};

}
}

#endif //PROPERTYPANEL_H
