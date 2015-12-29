/*
 * Vulkan Samples Kit
 *
 * Copyright (C) 2015 Valve Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
VULKAN_SAMPLE_SHORT_DESCRIPTION
create and destroy msg callback
*/

/*
 * This sample program registers a debug message callback
 * function and then intentionally causes an error in the
 * device limits debug layer to trigger the callback.
 *
 * The message callback is quiet by default so the sample
 * executes without emitting errors or creating a message box.
 *
 * Specifying the -noisy argument displays the message constructed
 * in the callback.
 */

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string.h>
#include "util.hpp"

#define APP_SHORT_NAME "vulkansamples_msgcallback"

typedef struct {
    bool callbackTriggered;
    bool callbackNoisy;
} UserData;

VkBool32 dbgFunc(
    VkFlags                             msgFlags,
    VkDbgObjectType                     objType,
    uint64_t                            srcObject,
    size_t                              location,
    int32_t                             msgCode,
    const char*                         pLayerPrefix,
    const char*                         pMsg,
    void*                               pUserDataIn)
{
    std::ostringstream message;
    UserData *pUserData = (UserData*)pUserDataIn;

    pUserData->callbackTriggered = true;

    message << "(Expected) ";
    if (msgFlags & VK_DBG_REPORT_ERROR_BIT) {
        message << "ERROR: ";
    } else if (msgFlags & VK_DBG_REPORT_WARN_BIT) {
        message << "WARNING: ";
    } else if (msgFlags & VK_DBG_REPORT_PERF_WARN_BIT) {
        message << "PERFORMANCE WARNING: ";
    } else if (msgFlags & VK_DBG_REPORT_INFO_BIT) {
        message << "INFO: ";
    } else if (msgFlags & VK_DBG_REPORT_DEBUG_BIT) {
        message << "DEBUG: ";
    }
    message << "[" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

    if (pUserData->callbackNoisy) {
#ifdef _WIN32
        MessageBox(NULL, message.str().c_str(), "Alert", MB_OK);
#else
        std::cout << message.str() << std::endl;
#endif
    }

    /*
     * false indicates that layer should not bail-out of an
     * API call that had validation failures. This may mean that the
     * app dies inside the driver due to invalid parameter(s).
     * That's what would happen without validation layers, so we'll
     * keep that behavior here.
     */
    return false;
}

int main(int argc, char **argv)
{
    VkExtensionProperties *vk_extension_props = NULL;
    uint32_t instance_extension_count;
    VkLayerProperties *vk_layer_props = NULL;
    uint32_t instance_layer_count;
    VkResult res;
    UserData userData = {};

    if (argc == 2 && !strcmp("-noisy", argv[1])) {
        userData.callbackNoisy = true;
    }

    /* VULKAN_KEY_START */

    /*
     * It's possible, though very rare, that the number of
     * instance layers could change. For example, installing something
     * could include new layers that the loader would pick up
     * between the initial query for the count and the
     * request for VkLayerProperties. If that happens,
     * the number of VkLayerProperties could exceed the count
     * previously given. To alert the app to this change
     * vkEnumerateInstanceExtensionProperties will return a VK_INCOMPLETE
     * status.
     * The count parameter will be updated with the number of
     * entries actually loaded into the data pointer.
     */

    do {
        res = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
        if (res)
            break;

        if (instance_extension_count == 0) {
            break;
        }

        vk_extension_props = (VkExtensionProperties *) realloc(vk_extension_props, instance_extension_count * sizeof(VkExtensionProperties));

        res = vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, vk_extension_props);
    } while (res == VK_INCOMPLETE);

    bool found_extension = false;
    for (uint32_t i = 0; i < instance_extension_count; i++) {
        if (!strcmp(vk_extension_props[i].extensionName, VK_DEBUG_REPORT_EXTENSION_NAME)) {
            found_extension = true;
        }
    }

    if (!found_extension) {
        std::cout << "Something went very wrong, cannot find DEBUG_REPORT extension" << std::endl;
        exit(1);
    }

    /*
     * Enable the Device Limits layer so that this program can intentionally generate
     * an error in this layer.
     *
     * Use the same approach as above, but look for the Device Limits layer.
     */

    do {
        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
        if (res)
            break;

        if (instance_layer_count == 0) {
            break;
        }

        vk_layer_props = (VkLayerProperties *) realloc(vk_layer_props, instance_layer_count * sizeof(VkLayerProperties));

        res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_layer_props);
    } while (res == VK_INCOMPLETE);

    bool found_layer = false;
    for (uint32_t i = 0; i < instance_layer_count; i++) {
        if (!strcmp(vk_layer_props[i].layerName, "VK_LAYER_LUNARG_DeviceLimits")) {
            found_layer = true;
        }
    }

    if (!found_layer) {
        std::cout << "Something went very wrong, cannot find VK_LAYER_LUNARG_DeviceLimits layer" << std::endl;
        exit(1);
    }

    const char *extension_names[1] = { VK_DEBUG_REPORT_EXTENSION_NAME };
    const char *layer_names[1] = { "VK_LAYER_LUNARG_DeviceLimits" };

    // initialize the VkApplicationInfo structure
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = APP_SHORT_NAME;
    app_info.applicationVersion = 1;
    app_info.pEngineName = APP_SHORT_NAME;
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION;

    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionNameCount = 1;
    inst_info.ppEnabledExtensionNames = extension_names;
    inst_info.enabledLayerNameCount = 1;
    inst_info.ppEnabledLayerNames = layer_names;

    VkInstance inst;

    res = vkCreateInstance(&inst_info, NULL, &inst);
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
        std::cout << "cannot find a compatible Vulkan ICD\n";
        exit(-1);
    } else if (res) {
        std::cout << "unknown error\n";
        exit(-1);
    }

    PFN_vkDbgCreateMsgCallback dbgCreateMsgCallback;
    PFN_vkDbgDestroyMsgCallback dbgDestroyMsgCallback;
    VkDbgMsgCallback msg_callback;

    dbgCreateMsgCallback = (PFN_vkDbgCreateMsgCallback) vkGetInstanceProcAddr(inst, "vkDbgCreateMsgCallback");
    if (!dbgCreateMsgCallback) {
        std::cout << "GetInstanceProcAddr: Unable to find vkDbgCreateMsgCallback function." << std::endl;
        exit(1);
    }

    dbgDestroyMsgCallback = (PFN_vkDbgDestroyMsgCallback) vkGetInstanceProcAddr(inst, "vkDbgDestroyMsgCallback");
    if (!dbgDestroyMsgCallback) {
        std::cout << "GetInstanceProcAddr: Unable to find vkDbgDestroyMsgCallback function." << std::endl;
        exit(1);
    }

    res = dbgCreateMsgCallback(
              inst,
              VK_DBG_REPORT_ERROR_BIT | VK_DBG_REPORT_WARN_BIT,
              dbgFunc,
              &userData,
              &msg_callback);
    switch (res) {
    case VK_SUCCESS:
        break;
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        std::cout << "dbgCreateMsgCallback: out of host memory pointer\n" << std::endl;
        exit(1);
        break;
    default:
        std::cout << "dbgCreateMsgCallback: unknown failure\n" << std::endl;
        exit(1);
        break;
    }

    /*
     * Purposely cause an error in the Device Limits layer to trigger the callback.
     *
     * Normally, an application enumerates physical devices by first calling this function with a NULL
     * pointer for the physical device(s) in order to get the count of physical devices.  After dynamically
     * allocating enough memory, the application calls the function again with the returned count and a pointer
     * to the memory just allocated.  Calling the function first with a non-NULL pointer creates an
     * invalid call sequence that is reported by the Device Limits layer.
     */

    uint32_t count = 1;
    VkPhysicalDevice physicalDevice;
    VkPhysicalDevice *pPhysicalDevices = &physicalDevice;
    res = vkEnumeratePhysicalDevices(inst, &count, pPhysicalDevices);

    // The callback should have been called because of the above error.
    if (!userData.callbackTriggered) {
#ifdef _WIN32
        MessageBox(NULL, "Message Callback did not get called.", "Alert", MB_OK);
#else
        std::cout << "Message Callback did not get called." << std::endl;
#endif
    }

    /* Clean up callback */
    dbgDestroyMsgCallback(inst, msg_callback);

    vkDestroyInstance(inst, NULL);

    /* VULKAN_KEY_END */

    return 0;
}
