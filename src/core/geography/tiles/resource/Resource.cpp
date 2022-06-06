//#include "Resource.hpp"
//
//#include <utility>
//
//#include "../GenericTile.hpp"
//
//namespace KCore {
//    Resource::Resource(std::function<void(GenericTile *)> processor) {
//        mPostprocessor = std::move(processor);
//    }
//
//    void Resource::invoke(GenericTile *tile) {
//        mPostprocessor(tile);
//    }
//}
