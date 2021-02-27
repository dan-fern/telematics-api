#include "testutils.hpp"
#include "templatehandler.hpp"

json constructHeader(const std::string& group_name) {
    TemplateHandler templates;
    json header = templates.getRawHeaderTemplate();
    header["group"] = group_name;
    return header;
}
