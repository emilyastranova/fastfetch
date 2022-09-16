#include "gpu.h"
#include "common/library.h"
#include "detection/cpu/cpu.h"
#include "util/apple/cfdict_helpers.h"

#include <IOKit/graphics/IOGraphicsLib.h>

const char* ffDetectGPUImpl(FFlist* gpus, const FFinstance* instance)
{
    FF_UNUSED(instance);

    CFMutableDictionaryRef matchDict = IOServiceMatching(kIOAcceleratorClassName);
    io_iterator_t iterator;
    if(IOServiceGetMatchingServices(0, matchDict, &iterator) != kIOReturnSuccess)
        return "IOServiceGetMatchingServices() failed";

    io_registry_entry_t registryEntry;
    while((registryEntry = IOIteratorNext(iterator)) != 0)
    {
        CFMutableDictionaryRef properties;
        if(IORegistryEntryCreateCFProperties(registryEntry, &properties, kCFAllocatorDefault, kNilOptions) != kIOReturnSuccess)
        {
            IOObjectRelease(registryEntry);
            continue;
        }

        FFGPUResult* gpu = ffListAdd(gpus);

        ffStrbufInitA(&gpu->vendor, 0);

        ffStrbufInit(&gpu->driver);
        ffCfDictGetString(properties, "CFBundleIdentifier", &gpu->driver);

        ffStrbufInit(&gpu->name);
        //IOAccelerator returns model property for Apple Silicon, but not for Intel Iris GPUs.
        //Still needs testing for AMD's
        if(!ffCfDictGetString(properties, "model", &gpu->name) && gpu->driver.length > 0)
            ffStrbufAppendS(&gpu->name, gpu->driver.chars + ffStrbufLastIndexC(&gpu->driver, '.') + 1);

        if(!ffCfDictGetInt(properties, "gpu-core-count", &gpu->coreCount))
            gpu->coreCount = FF_GPU_CORE_COUNT_UNSET;

        gpu->temperature = FF_GPU_TEMP_UNSET;

        CFRelease(properties);
        IOObjectRelease(registryEntry);
    }

    IOObjectRelease(iterator);
    return NULL;
}
