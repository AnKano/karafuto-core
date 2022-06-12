# Karafuto Core

<p align="center">
    <img src="github-assets/logo.png" height="300" />
</p>

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean lobortis lorem vitae metus suscipit, fringilla auctor
nunc sodales. Curabitur ultricies molestie mi, et rutrum enim rhoncus id. Fusce eu dui cursus, dictum nisl eget,
hendrerit augue. In fermentum sed turpis et vehicula. Sed ornare nisi sit amet cursus condimentum. Nullam quis mollis
turpis. Nam tincidunt orci sem, id aliquet justo pharetra a. Pellentesque habitant morbi tristique senectus et netus et
malesuada fames ac turpis egestas. Maecenas semper augue diam, ut luctus justo tempor non. Pellentesque aliquet risus
dolor, et posuere turpis fringilla a. Phasellus et ultricies justo. Cras a condimentum libero.

### Shortcuts

- [Based on](#based-on)
- [How it works](#how-it-works)

## Based on

- GLM mathematic library
- HTTPRequest
- lrucache17
- rapidjson
- stb-image
- GPU APIs
    - Vulkan
    - OpenCL (1.2, for GPUs that not support Vulkan for any reasons)

Special thanks for [ViziCities](https://github.com/UDST/vizicities) by @robhawkes

## How it works

### Space subdivision

Segments can be setuped manually:

Lorem ipsum dolor sit amet, consectetur adipiscing elit

<img src="github-assets/1.png" height="150" />

Lorem ipsum dolor sit amet, consectetur adipiscing elit

<img src="github-assets/1-1.png" height="150" />

### Layer core



### Subdivision modes

|                        Naive subdivision                        | Two-pass subdivision                           |
|:---------------------------------------------------------------:|------------------------------------------------|
|         <img src="github-assets/1.png" height="150" />          | <img src="github-assets/2.png" height="150" /> |
|     Lorem ipsum dolor sit amet, consectetur adipiscing elit     | Aenean lobortis lorem vitae metus suscipit |
| <img src="github-assets/subdivision-mode-1.png" height="150" /> | <img src="github-assets/2-vulkan-chunks.png" height="150" /> |


```

```

 <img src="github-assets/consolidated.png" height="150" />

### Elevation sources


```
// process GeoJSON file and try to get elevation value from related SRTM source 
SRTMElevation* CreateSRTMElevationRepo();
 
// process GeoJSON file and without elevation search  
DllExport void SRTMElevationRepoAddSource(SRTMElevation* srcPtr, const char* path, SourceType type);

<...>

// SRTM source can be one from types listed below
enum SourceType {
    SourceFile,         // as file path
    SourceUrl           // as url path
};
```

### Tile meshes with elevation

|                      8 Segments                       |                     16   Segments                      |                      128 Segments                       |
|:-----------------------------------------------------:|--------------------------------------------------------|:-------------------------------------------------------:|
| <img src="github-assets/terrain8.png" height="150" /> | <img src="github-assets/terrain16.png" height="150" /> | <img src="github-assets/terrain128.png" height="150" /> |

```
// create tile using TMS-code with count of segments passed in 5 and 6 arguements  
DllExport GridMesh *CreateTileMeshXYZ(IElevationSrc *srcPtr, uint8_t zoom,
                                      uint16_t x, uint16_t y,
                                      uint16_t slicesX, uint16_t slicesY,
                                      bool flipUVsX, bool flipUVsY);
                                      
// create tile using Quadcode with count of segments passed in 3 and 4 arguements  
DllExport GridMesh *CreateTileMeshQuadcode(IElevationSrc *srcPtr, const char *quadcode,
                                           uint16_t slicesX, uint16_t slicesY,
                                           bool flipUVsX, bool flipUVsY);
```

### GeoJSON meshes

<img src="github-assets/primitives-without-adaptation.png" height="150" /> 
<img src="github-assets/primitives-adaptation.png" height="150" /> 

```
// process GeoJSON file and try to get elevation value from related SRTM source 
DllExport std::vector<GeoJSONTransObject> *
ProcessGeoJSONWithElevation(LayerInterface *layerPtr, IElevationSrc *elevation, GeoJSONSourceType type, const char *param);
 
// process GeoJSON file and without elevation search  
DllExport std::vector<GeoJSONTransObject> *
ProcessGeoJSON(LayerInterface *layerPtr, GeoJSONSourceType type, const char *param);

<...>

// GeoJSON source can be one from types listed below
enum GeoJSONSourceType {
    GeoJSONRaw,         // as string text 
    GeoJSONFile,        // as file path
    GeoJSONUrl          // as url path
};
```

### Combination

<img src="github-assets/additional-1.png" width="600">

## TODO

- :black_square_button: - Remake CMake for MacOS and Linux
- :black_square_button: - Make it compatible with iOS
- :black_square_button: - GeoTIFF
- :black_square_button: - Add tests for proving model accuracy 

## License

##  