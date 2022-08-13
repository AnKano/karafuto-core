//#include <chrono>
//#include <glm/glm.hpp>
//#include <utility>
//
//#include "layer/LayerInterface.hpp"
//#include "resources/primitives/GeoJSONProcessor.hpp"
//
//#include "resources/elevation/IElevationSrc.hpp"
//#include "resources/elevation/srtm/SRTMElevation.hpp"
//#include "resources/ISource.hpp"
//
//int main() {
//    auto elevationSrc = KCore::CreateSRTMElevationRepo();
//
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N45E141.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N45E142.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N46E142.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N46E142.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N46E143.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N47E142.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N47E142.hgt", KCore::SourceUrl);
//    KCore::SRTMElevationRepoAddSource(elevationSrc, "http://192.168.0.6:8000/N47E143.hgt", KCore::SourceUrl);
//
//    auto mesh = KCore::CreateTileMeshXYZ(elevationSrc, 10, 918, 360, 2, 2);
////    auto mesh2 = KCore::CreateTileMeshQuadcode(elevationSrc, "1312201", 64, 64);
//
//    int a = 0;

//    KCore::SRTMElevation elevationSrc;
//
//    //declare sources
//    {
//        elevationSrc.addSourcePart(new KCore::SRTMSource("http://192.168.0.6:8000/N45E141.hgt", KCore::SourceType::SourceUrl));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N45E142.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N46E142.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N46E142.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N46E143.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N47E142.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N47E142.hgt", KCore::SourceType::SourceFile));
//        elevationSrc.addSourcePart(new KCore::SRTMSource("assets/sources/N47E143.hgt", KCore::SourceType::SourceFile));
//    }
//
//    auto mesh = elevationSrc.createTile(10, 918, 360, 8, 8);
//
//    int a = 0;

//    const uint16_t viewportWidth{1920}, viewportHeight{1080};
//    const float aspectRatio{(float) viewportWidth / viewportHeight};
//
//    // create camera that describe point of view and matrix
//    glm::mat4 cameraProjectionMatrix;
//    glm::mat4 cameraViewMatrix;
//
//    glm::vec3 cameraOpenGlSpacePosition{1000.0f, 10000.0f, 10000.0f};
//    glm::vec3 cameraOpenGlSpaceTarget{0.0f, 0.0f, 0.0f};
//    glm::vec3 cameraOpenGlSpaceUp{0.0f, 1.0f, 0.0f};
//
//    // setup matrices
//    cameraViewMatrix = glm::lookAt(cameraOpenGlSpacePosition, cameraOpenGlSpaceTarget, cameraOpenGlSpaceUp);
//    cameraProjectionMatrix = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 2500000.0f);
//
//    auto *core = KCore::CreateTileLayerOSM(46.9181f, 142.7189f);
//
//    auto b = KCore::ProcessGeoJSONFile(core, "assets/sources/12.geojson");

//    const uint32_t iterations{1000000};
//    for (auto i = 0; i < iterations; i++) {
//        cameraOpenGlSpacePosition.x -= 10.0f;
//        KCore::UpdateLayer(
//                core,
//                reinterpret_cast<float *>(&cameraProjectionMatrix),
//                reinterpret_cast<float *>(&cameraViewMatrix),
//                reinterpret_cast<float *>(&cameraOpenGlSpacePosition)
//        );
//        auto a = KCore::GetEventsVector(core);
//        for (const auto &item: *a) {
//            switch (item.type) {
//                case KCore::InFrustum: {
//                    auto *body = static_cast<KCore::TileDescription *>(item.payload);
////                    std::cout << "In frustum " << item.quadcode << std::endl;
//                    break;
//                }
//                case KCore::NotInFrustum:
////                    std::cout << "Not In frustum " << item.quadcode << std::endl;
//                    break;
//                default:
//                    break;
//            }
//            std::cout << std::endl;
//        }
//    }

//    return 0;
//}