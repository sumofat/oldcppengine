
IMGUI_IMPL_API bool ImGui_ImplMetal_Init(RenderDevice device,RenderPassDescriptor* renderpassdesc);
//IMGUI_IMPL_API void ImGui_ImplMetal_Shutdown();
IMGUI_IMPL_API void ImGui_ImplMetal_NewFrame(RenderPassDescriptor *renderPassDescriptor);
IMGUI_IMPL_API void ImGui_ImplMetal_RenderDrawData(ImDrawData* draw_data, void* commandBuffer, RenderCommandEncoder* commandEncoder);

// Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool ImGui_ImplMetal_CreateFontsTexture(RenderDevice device);
//IMGUI_IMPL_API void ImGui_ImplMetal_DestroyFontsTexture();
//IMGUI_IMPL_API bool ImGui_ImplMetal_CreateDeviceObjects(RenderDevice device);
//IMGUI_IMPL_API void ImGui_ImplMetal_DestroyDeviceObjects();
IMGUI_IMPL_API void ImGui_ImplOSX_NewFrame();
