#include "../esp_flasher_app_i.h"

static void esp_flasher_scene_browse_var_list_enter_callback(void* context, uint32_t index) {
    EspFlasherApp* app = context;
    furi_assert(index < NUM_FLASH_OPTIONS);
    const char* ext_filter = app->chip_type == ChipTypeRTL ? "*.bin" : "*.bin";
    
    if(true) {
        FuriString* tmp_file_path = furi_string_alloc();
        furi_string_set_str(tmp_file_path, ESP_APP_FOLDER);

        DialogsFileBrowserOptions browser_options;
        dialog_file_browser_set_basic_options(&browser_options, ext_filter, &I_esp_flasher_10px);
        browser_options.base_path = ESP_APP_FOLDER;

        if(dialog_file_browser_show(app->dialogs, tmp_file_path, tmp_file_path, &browser_options)) {
            if(app->chip_type == ChipTypeRTL && index == SelectedFlashRTLBin) {
                strncpy(app->bin_file_path_rtl, furi_string_get_cstr(tmp_file_path), 
                       sizeof(app->bin_file_path_rtl) - 1);
            } else {
                // Handle ESP files
                switch(index) {
                    case SelectedFlashBoot:
                        strncpy(app->bin_file_path_boot, furi_string_get_cstr(tmp_file_path), 
                               sizeof(app->bin_file_path_boot) - 1);
                        break;
                    case SelectedFlashPart:
                        strncpy(app->bin_file_path_part, furi_string_get_cstr(tmp_file_path), 
                               sizeof(app->bin_file_path_part) - 1);
                        break;
                    case SelectedFlashAppA:
                        strncpy(app->bin_file_path_app_a, furi_string_get_cstr(tmp_file_path), 
                               sizeof(app->bin_file_path_app_a) - 1);
                        break;
                    case SelectedFlashCustom:
                        strncpy(app->bin_file_path_custom, furi_string_get_cstr(tmp_file_path), 
                               sizeof(app->bin_file_path_custom) - 1);
                        break;
                    default:
                        break;
                }
            }
        }
        furi_string_free(tmp_file_path);
    }
}

static void esp_flasher_scene_browse_var_list_change_callback(VariableItem* item) {
    uint32_t selected_option_index = (uint32_t)(uintptr_t)variable_item_get_context(item);
    
    // Get the app context from the variable item list
    VariableItemList* var_item_list = variable_item_get_parent(item);
    void* list_context = variable_item_list_get_context(var_item_list);
    EspFlasherApp* app = (EspFlasherApp*)list_context;
    
    uint8_t index = variable_item_get_current_value_index(item);

    if(selected_option_index < NUM_FLASH_OPTIONS) {
        app->selected_flash_options[selected_option_index] = index == 1;

        // Update the flash options counter
        app->num_selected_flash_options = 0;
        for(int i = 0; i < NUM_FLASH_OPTIONS; i++) {
            if(app->selected_flash_options[i]) {
                app->num_selected_flash_options++;
            }
        }
        
        // Update the display text
        variable_item_set_current_value_text(item, index == 1 ? "Yes" : "No");
    }
}

void esp_flasher_scene_browse_on_enter(void* context) {
    EspFlasherApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, esp_flasher_scene_browse_var_list_enter_callback, app);
    
    // Set the context for the variable item list so we can access it in callbacks
    variable_item_list_set_context(var_item_list, app);

    VariableItem* item;

    if(app->chip_type == ChipTypeESP) {
        // ESP chip options
        item = variable_item_list_add(var_item_list, "Bootloader", 2, esp_flasher_scene_browse_var_list_change_callback, (void*)(uintptr_t)SelectedFlashBoot);
        variable_item_set_current_value_index(item, app->selected_flash_options[SelectedFlashBoot]);
        variable_item_set_current_value_text(item, app->selected_flash_options[SelectedFlashBoot] ? "Yes" : "No");

        item = variable_item_list_add(var_item_list, "Partition table", 2, esp_flasher_scene_browse_var_list_change_callback, (void*)(uintptr_t)SelectedFlashPart);
        variable_item_set_current_value_index(item, app->selected_flash_options[SelectedFlashPart]);
        variable_item_set_current_value_text(item, app->selected_flash_options[SelectedFlashPart] ? "Yes" : "No");

        item = variable_item_list_add(var_item_list, "App A", 2, esp_flasher_scene_browse_var_list_change_callback, (void*)(uintptr_t)SelectedFlashAppA);
        variable_item_set_current_value_index(item, app->selected_flash_options[SelectedFlashAppA]);
        variable_item_set_current_value_text(item, app->selected_flash_options[SelectedFlashAppA] ? "Yes" : "No");

        item = variable_item_list_add(var_item_list, "Custom", 2, esp_flasher_scene_browse_var_list_change_callback, (void*)(uintptr_t)SelectedFlashCustom);
        variable_item_set_current_value_index(item, app->selected_flash_options[SelectedFlashCustom]);
        variable_item_set_current_value_text(item, app->selected_flash_options[SelectedFlashCustom] ? "Yes" : "No");
    } else {
        // RTL chip options
        item = variable_item_list_add(var_item_list, "RTL Binary", 2, esp_flasher_scene_browse_var_list_change_callback, (void*)(uintptr_t)SelectedFlashRTLBin);
        variable_item_set_current_value_index(item, app->selected_flash_options[SelectedFlashRTLBin]);
        variable_item_set_current_value_text(item, app->selected_flash_options[SelectedFlashRTLBin] ? "Yes" : "No");
    }

    // Flash button
    item = variable_item_list_add(var_item_list, "[>] FLASH", 0, NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, EspFlasherAppViewVarItemList);
}

bool esp_flasher_scene_browse_on_event(void* context, SceneManagerEvent event) {
    EspFlasherApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == EspFlasherCustomEventVarItemListIndexChanged) {
            // Check if it's the flash button being pressed
            size_t selected_index = variable_item_list_get_selected_item_index(app->var_item_list);
            size_t expected_flash_index = app->chip_type == ChipTypeRTL ? 1 : 4; // Based on number of options
            
            if(selected_index == expected_flash_index) {
                // Start flashing
                scene_manager_next_scene(app->scene_manager, EspFlasherSceneConsoleOutput);
            }
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void esp_flasher_scene_browse_on_exit(void* context) {
    EspFlasherApp* app = context;
    variable_item_list_reset(app->var_item_list);
}