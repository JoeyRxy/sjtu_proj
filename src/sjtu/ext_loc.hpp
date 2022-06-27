#pragma once

#include "hmm/location.hpp"

namespace rxy {

class ExtLocation : public Location {
   private:
    int ext_id;

   public:
    ExtLocation(int id, int ext_id, Point const &point) : Location(id, point), ext_id(ext_id) {}
    ExtLocation(int id, int ext_id, Point &&point) : Location(id, std::move(point)), ext_id(ext_id) {}

    size_t get_hash() const override { return ext_id; }

    bool operator==(Location const &rhs) const override {
        if (!Location::operator==(rhs)) return false;
        try {
            return ext_id == dynamic_cast<ExtLocation const &>(rhs).ext_id;
        } catch (std::bad_cast const &) {
            return false;
        }
    }
};

using ExtLocPtr = std::shared_ptr<ExtLocation>;

}  // namespace rxy
