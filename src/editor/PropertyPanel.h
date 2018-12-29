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
                    std::function<T (KeyRef)> maxGetter);

    model::Fade fade;
    bool showSlider(const char *name, int max, int &item) const;
    bool showSlider(const char *name, float max, float &item) const;
};

}
}

#endif //PROPERTYPANEL_H
