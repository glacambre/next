//
// Copyright © 2017-2018 Atlas Engineer LLC.
// Use of this file is governed by the license that can be found in LICENSE.
//

#import "Server.h"
#import "NextApplicationDelegate.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>
#include <xmlrpc-c/config.h>

@implementation Server

static xmlrpc_value *
window_make(xmlrpc_env *   const envP,
            xmlrpc_value * const paramArrayP,
            void *         const serverInfo,
            void *         const channelInfo) {
    __block NSString* operationResult = @"";
    dispatch_sync(dispatch_get_main_queue(), ^{
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate windowMake];
    });
    return xmlrpc_build_value(envP, "s", [operationResult UTF8String]);
}

static xmlrpc_value *
window_delete(xmlrpc_env * const envP,
            xmlrpc_value * const paramArrayP,
            void *         const serverInfo,
            void *         const channelInfo) {
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *windowId;
        xmlrpc_decompose_value(envP, paramArrayP, "(s)", &windowId);
        NextApplicationDelegate *delegate = [NSApp delegate];
        [delegate windowDelete: [NSString stringWithFormat:@"%s", windowId]];
    });
    return xmlrpc_bool_new(envP, 1);
}

static xmlrpc_value *
window_active(xmlrpc_env *   const envP,
            xmlrpc_value * const paramArrayP,
            void *         const serverInfo,
            void *         const channelInfo) {
    __block NSString* operationResult = @"";
    dispatch_sync(dispatch_get_main_queue(), ^{
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate windowActive];
    });
    return xmlrpc_build_value(envP, "s", [operationResult UTF8String]);
}

static xmlrpc_value *
window_set_active_buffer(xmlrpc_env *   const envP,
              xmlrpc_value * const paramArrayP,
              void *         const serverInfo,
              void *         const channelInfo) {
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *windowId;
        void *bufferId;
        xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &windowId, &bufferId);
        NextApplicationDelegate *delegate = [NSApp delegate];
        [delegate setActiveBufferForWindow:[NSString stringWithFormat:@"%s", windowId]
                                withBuffer: [NSString stringWithFormat:@"%s", bufferId]];
    });
    return xmlrpc_bool_new(envP, 1);
}

static xmlrpc_value *
buffer_make(xmlrpc_env *   const envP,
            xmlrpc_value * const paramArrayP,
            void *         const serverInfo,
            void *         const channelInfo) {
    __block NSString* operationResult = @"";
    dispatch_sync(dispatch_get_main_queue(), ^{
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate bufferMake];
    });
    return xmlrpc_build_value(envP, "s", [operationResult UTF8String]);
}

static xmlrpc_value *
buffer_delete(xmlrpc_env * const envP,
              xmlrpc_value * const paramArrayP,
              void *         const serverInfo,
              void *         const channelInfo) {
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *bufferId;
        xmlrpc_decompose_value(envP, paramArrayP, "(s)", &bufferId);
        NextApplicationDelegate *delegate = [NSApp delegate];
        [delegate bufferDelete: [NSString stringWithFormat:@"%s", bufferId]];
    });
    return xmlrpc_bool_new(envP, 1);
}

static xmlrpc_value *
buffer_execute_javascript(xmlrpc_env *   const envP,
                          xmlrpc_value * const paramArrayP,
                          void *         const serverInfo,
                          void *         const channelInfo) {
    __block NSString* operationResult = @"";
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *bufferId;
        void *javaScript;
        xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &bufferId, &javaScript);
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate bufferExecuteJavaScript:[NSString stringWithFormat:@"%s", bufferId]
                                             withJavaScript:[NSString stringWithFormat:@"%s", javaScript]];
    });
    return xmlrpc_build_value(envP, "s", [operationResult UTF8String]);
}

static xmlrpc_value *
minibuffer_set_height(xmlrpc_env *   const envP,
                      xmlrpc_value * const paramArrayP,
                      void *         const serverInfo,
                      void *         const channelInfo) {
    __block NSInteger operationResult = 0;
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *windowId;
        xmlrpc_int minibufferHeight;
        xmlrpc_decompose_value(envP, paramArrayP, "(si)", &windowId, &minibufferHeight);
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate minibufferSetHeight:minibufferHeight
                                              forWindow:[NSString stringWithFormat:@"%s", windowId]];
    });
    return xmlrpc_int_new(envP, (int)operationResult);
}

static xmlrpc_value *
minibuffer_execute_javascript(xmlrpc_env *   const envP,
                               xmlrpc_value * const paramArrayP,
                               void *         const serverInfo,
                               void *         const channelInfo) {
    __block NSString* operationResult = @"";
    dispatch_sync(dispatch_get_main_queue(), ^{
        void *windowId;
        void *javaScript;
        xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &windowId, &javaScript);
        NextApplicationDelegate *delegate = [NSApp delegate];
        operationResult = [delegate minibufferExecuteJavaScript:[NSString stringWithFormat:@"%s", windowId]
                                             withJavaScript:[NSString stringWithFormat:@"%s", javaScript]];
    });
    return xmlrpc_string_new(envP, [operationResult UTF8String]);
}

- (void) start {
    struct xmlrpc_method_info3 const windowMake = {
        "window.make", &window_make,};
    struct xmlrpc_method_info3 const windowDelete = {
        "window.delete", &window_delete,};
    struct xmlrpc_method_info3 const windowActive = {
        "window.active", &window_active,};
    struct xmlrpc_method_info3 const windowSetActiveBuffer = {
        "window.set.active.buffer", &window_set_active_buffer,};
    struct xmlrpc_method_info3 const bufferMake = {
        "buffer.make", &buffer_make,};
    struct xmlrpc_method_info3 const bufferDelete = {
        "buffer.delete", &buffer_delete,};
    struct xmlrpc_method_info3 const bufferExecuteJavaScript = {
        "buffer.execute.javascript", &buffer_execute_javascript,};
    struct xmlrpc_method_info3 const minibufferSetHeight = {
        "minibuffer.set.height", &minibuffer_set_height,};
    struct xmlrpc_method_info3 const minibufferExecuteJavascript = {
        "minibuffer.execute.javascript", &minibuffer_execute_javascript,};

    xmlrpc_server_abyss_parms serverparm;
    xmlrpc_registry * registryP;
    xmlrpc_env env;
    xmlrpc_env_init(&env);
    
    registryP = xmlrpc_registry_new(&env);
    if (env.fault_occurred) {
        printf("xmlrpc_registry_new() failed.  %s\n", env.fault_string);
        exit(1);
    }
    
    xmlrpc_registry_add_method3(&env, registryP, &windowMake);
    xmlrpc_registry_add_method3(&env, registryP, &windowDelete);
    xmlrpc_registry_add_method3(&env, registryP, &windowActive);
    xmlrpc_registry_add_method3(&env, registryP, &windowSetActiveBuffer);
    xmlrpc_registry_add_method3(&env, registryP, &bufferMake);
    xmlrpc_registry_add_method3(&env, registryP, &bufferDelete);
    xmlrpc_registry_add_method3(&env, registryP, &bufferExecuteJavaScript);
    xmlrpc_registry_add_method3(&env, registryP, &minibufferSetHeight);
    xmlrpc_registry_add_method3(&env, registryP, &minibufferExecuteJavascript);

    if (env.fault_occurred) {
        printf("xmlrpc_registry_add_method3() failed.  %s\n",
               env.fault_string);
        exit(1);
    }
    
    serverparm.config_file_name = NULL;   /* Select the modern normal API */
    serverparm.registryP        = registryP;
    serverparm.port_number      = 8082;
    serverparm.log_file_name    = "/tmp/next_xmlrpc_log";
        
    xmlrpc_server_abyss(&env, &serverparm, XMLRPC_APSIZE(log_file_name));
    if (env.fault_occurred) {
        printf("xmlrpc_server_abyss() failed.  %s\n", env.fault_string);
        exit(1);
    }
}


@end