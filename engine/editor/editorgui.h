#if !defined(EDITORGUI_H)

//NOTE(Ray):In order the editor to ahem edit we need references to rest of the engine
//and systems which creates entanglement... 
namespace EditorGUI
{
//TODO(Ray):Deffer some commands // maybe not? thinking about it.
    YoyoVector imgui_commands;

    bool show_asset_library_window = false;
    bool show_reference_dummy_window = false;
    //NOTE(Ray):Thinking of wrapping and deffering these for various reasons.
    //Only reason I am now doing it right away is perf and code complexity reasons.
    //Otherise I think its the best approach. A lil tricky tho

    //NOTE(Ray):A window that will give us visibilty into the asset system.
    //We should have access to all the assets the engine knows about here.
    //We will:
    //1. Set the asset data folder location to look at. 
    //2. Be able Load them delete import new ones etc...
    //3. Preview
    void ShowAssetLibraryWindow()
    {
        ImGui::Begin("Asset Library Window");                          // Create a window called "Hello, world!" and append into it.
        //1. Organized asset browser
         //a. searchable
         //b. filterable
         //c. show by directory or by groups
        //2. Side window or companion window will have a preview of the asset
          //a. texture
          //b. model (with or without ref textures)
          //c. can also show gameobject
          //d. Animation (wih default bot or with other asset)

        //Get all files in data dir
        //make list for models textures etc... get the settings for each file
        {
            static bool disable_menu;
            static bool disable_mouse_wheel;
            static ImTextureID current_tex_id;
            static LoadedTexture current_texture;
            // Child 2: rounded border
            {
                if (ImGui::Button("Select.."))
                    ImGui::OpenPopup("my_select_popup");
                
                ImGuiWindowFlags window_flags = (disable_mouse_wheel ? ImGuiWindowFlags_NoScrollWithMouse : 0) | (disable_menu ? 0 : ImGuiWindowFlags_MenuBar);
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::BeginChild("Child2", ImVec2(0, 260));
                if (!disable_menu && ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("Menu"))
                    {
                        ShowExampleMenuFile();
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                
                dir_files_result result = PlatformGetAllAssetFilesInDir(Directory_None,&StringsHandler::transient_string_memory,true,true);
                int i =0;
                file_info* f_info;
                while ((f_info = YoyoIterateVector(&result.Files, file_info)))
                {
                    if(ImGui::Button(f_info->Name.String, ImVec2(-1.0f, 0.0f)))
                    {
                        LoadedTexture tex;
                        Yostr file_ext = GetExtension(&f_info->Name,&StringsHandler::transient_string_memory,false);
                        MetaFileType::Type type = MetaFiles::GetFileExtensionType(file_ext);
                        if(MetaFileType::PNG == type || MetaFileType::PSD == type)
                        {
                            if(AssetSystem::AddOrGetTexture(f_info->Name,&tex))
                            {
                                current_texture = tex;
                                current_tex_id = tex.texture.state;
                            }
                            //result = CreateDefaultModelMetaFile(file,model);
                        }
                       
                    }
//                ImGui::Text("%04d: %s", i++,f_info->Name.String);
//                    PlatformOutput(true, f_info->Name.String);
                }
                YoyoFreeVectorMem(&result.Files);
                ImGui::EndChild();
                ImGui::PopStyleVar();
            }
            
            ImGui::Separator();//--------

//            if (ImGui::TreeNode("Images"))
            {
                ImGuiIO& io = ImGui::GetIO();
                ImTextureID my_tex_id = current_tex_id;//io.Fonts->TexID;
                float my_tex_w = current_texture.dim.x();//(float)io.Fonts->TexWidth;
                float my_tex_h = current_texture.dim.y();//(float)io.Fonts->TexHeight;
                float ww = ImGui::GetWindowWidth() - 15;
                float scale = ww / my_tex_w;
                
                ImGui::Text("%.0fx%.0f", my_tex_w, my_tex_h);
                ImVec2 pos = ImGui::GetCursorScreenPos();
                
                ImGui::Image(my_tex_id, ImVec2(my_tex_w * scale, my_tex_h * scale), ImVec2(0,0), ImVec2(1,1), ImColor(255,255,255,255), ImColor(255,255,255,128));
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    float region_sz = 32.0f;
                    float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > my_tex_w - region_sz) region_x = my_tex_w - region_sz;
                    float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > my_tex_h - region_sz) region_y = my_tex_h - region_sz;
                    float zoom = 4.0f;
                    ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
                    ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
                    ImVec2 uv0 = ImVec2((region_x) / my_tex_w, (region_y) / my_tex_h);
                    ImVec2 uv1 = ImVec2((region_x + region_sz) / my_tex_w, (region_y + region_sz) / my_tex_h);
                    ImGui::Image(my_tex_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImColor(255,255,255,255), ImColor(255,255,255,128));
                    ImGui::EndTooltip();
                }
//                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    void ShowTheMainMenuBar()
    {
        if(ImGui::BeginMainMenuBar())
        {
            //For saving scenes edited settings and whatever 
            if (ImGui::BeginMenu("Serialization"))
            {
//                ShowExampleMenuFile();
                ImGui::EndMenu();
            }            
            if (ImGui::BeginMenu("Assets"))
            {
                if (ImGui::MenuItem("AssetLibraryWindow", "CTRL+A"))
                {
                    show_asset_library_window = ~show_asset_library_window;
                }
                ImGui::Separator();//Seperate the window from the rest of the options.
                
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Debug"))
            {
                if(ImGui::MenuItem("Show Demo Window for reference"))
                {
                    show_reference_dummy_window = ~show_reference_dummy_window;
                }
                ImGui::EndMenu();
            }
//            ImGui::Separator();
//            ImGui::PushItemWidth(800);
//            float f = 100;
//            ImGui::DragFloat("float##1", &f);
//            ImGui::PushItemWidth(ImGui::GetWindowWidth());
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::EndMainMenuBar();            
        }

        //Windows
        if(show_asset_library_window)
        {
            ShowAssetLibraryWindow();
        }
        
        if(show_reference_dummy_window)
        {
            ImGui::ShowDemoWindow(&show_reference_dummy_window);
        }
    }

    void Init()
    {
    }
    
    void Update()
    {
        ShowTheMainMenuBar();        
    }
}
/*imgui ref code delete later
#if 0
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

        
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            
            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        
        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }
#endif
*/

#define EDITORGUI_H
#endif
