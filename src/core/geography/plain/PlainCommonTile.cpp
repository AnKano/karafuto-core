//
// Created by anshu on 1/29/2022.
//


#include "PlainCommonTile.hpp"

namespace KCore {
    PlainCommonTile::PlainCommonTile(CommonTile *common) : mDescription(common->getDescription().mPayload) {
        mMesh = common->getMesh().get();
        mImage = common->getImage().get();
    }

    void PlainCommonTile::dispose() const {
        delete mMesh;
        delete mImage;
    }

    uint8_t *GetImageBytes(std::vector<uint8_t> *image, int &length) {
        length = (int) image->size();
        return const_cast<uint8_t *>(image->data());
    }
}
