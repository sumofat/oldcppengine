//TODO(Ray):We will have the ability to have mutiple data folders.
//from the editor and engine perspective its all under one data folder but when
//doing searches and quires for assets we scan all the defined data directories...
//all assets loaded from the scene will be added to a watch list and refreshed automatically
//when a save file is detected.
//1.  This allows for games to share asset libraries or not share them at will.
//2.  A password could be given or at the network access level.
//3. the data path could be a network drive or computer on the net or anything...

#if ENGINEIMPL

namespace AssetSystem
{
    FbxManager* fbx_manager = NULL;
    FbxScene* fbx_scene = NULL;

    bool asset_system_log = true;

    Yostr error_strings[1000];
    u32 error_count = 0;

    AnythingCache texture_cache;
    AnythingCache gl_texture_cache;
    RenderMaterial default_mat;
    VertexDescriptor default_vertex_descriptor;
    
    AnythingCache material_cache;

    
//    YoyoVector renderable_asset_table;
    AnythingCache render_asset_cache;
    
    void InitDefaultMaterial()
    {
//hardcoded default material

#if 0
        Yostr d_vs_name = CreateStringFromLiteral("diffuse_vs", &StringsHandler::transient_string_memory);
        Yostr d_fs_name = CreateStringFromLiteral("diffuse_color_fs", &StringsHandler::transient_string_memory);
#else
        Yostr d_vs_name = CreateStringFromLiteral("diffuse_vs", &StringsHandler::transient_string_memory);
        Yostr d_fs_name = CreateStringFromLiteral("diffuse_fs", &StringsHandler::transient_string_memory);
#endif
        float4 d_base_color = float4(0.5f,0.5f,0.5f,1.0f);
        Yostr d_mat_string = CreateStringFromLiteral("standard_hardcoded_default_material",&StringsHandler::transient_string_memory);
        uint64_t d_mat_key = StringsHandler::StringHash(d_mat_string.String,d_mat_string.Length);
        default_mat = AssetSystem::CreateMaterialFromDescription(&d_vs_name,&d_fs_name,d_base_color);
        AnythingCacheCode::AddThing(&material_cache,&d_mat_key,&default_mat);
    }
    
    Yostr GetDataPath(const char* file,MemoryArena* arena)
    {
        Yostr path = BuildPathToAssets(arena,0);
        Yostr FinalPathToAsset = AppendString(path,CreateStringFromLiteral(file,arena),arena);
        return FinalPathToAsset;
    }
    
    void InitializeSdkObjects()
    {
        //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
        fbx_manager = FbxManager::Create();
        if (!fbx_manager)
        {
            PlatformOutput(asset_system_log, "Error: Unable to create FBX Manager!\n");
            exit(1);
        }
        else
        {
            PlatformOutput(asset_system_log, "Autodesk FBX SDK version %s\n", fbx_manager->GetVersion());
        }

        
        //Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(fbx_manager, IOSROOT);
        fbx_manager->SetIOSettings(ios);

        //Load plugins from the executable directory (optional)
        FbxString lPath = FbxGetApplicationDirectory();
        fbx_manager->LoadPluginsDirectory(lPath.Buffer());

        //Create an FBX scene. This object holds most objects imported/exported from/to files.
        fbx_scene = FbxScene::Create(fbx_manager, "My Scene");
        if (!fbx_scene)
        {
            PlatformOutput(asset_system_log, "Error: Unable to create FBX scene!\n");
            exit(1);
        }
    }
    
    void Init()
    {
        InitializeSdkObjects();

        AnythingCacheCode::Init(&material_cache,3000,sizeof(RenderMaterial),sizeof(uint64_t));
        InitDefaultMaterial();

        //TextureCache::Init(3000);
        //ModelCache::Init(3000);
        GPUResourceCache::Init(3000);
        AnythingCacheCode::Init(&texture_cache,3000,sizeof(LoadedTexture),sizeof(uint64_t));
        AnythingCacheCode::Init(&gl_texture_cache,3000,sizeof(GLTexture),sizeof(uint64_t));
        
        AnythingCacheCode::Init(&render_asset_cache,3000,sizeof(ModelAsset),sizeof(RenderAssetKey));
    }

    void UploadModelAssetToOpenGLEmuState(OpenGLEmuState*s,ModelAsset* ma)
    {
        for(int i = 0;i < ma->meshes.count;++i)
        {
            int is_valid = 0;
            GPUMeshResource mesh_r;
            MeshAsset* mesh = (MeshAsset*)ma->meshes.base + i;
            if(mesh->vertex_count > 0)
            {
                u64 v_size = sizeof(float) * mesh->vertex_count;
                mesh_r.vertex_buff = ogle_gen_buffer(s,v_size,ResourceStorageModeShared);
                ogle_buffer_data_named(s,v_size,&mesh_r.vertex_buff,(void*)mesh->vertex_data);
                is_valid++;
            }

            if(mesh->normal_count > 0)
            {
                u64 size = sizeof(float) * mesh->normal_count;
                mesh_r.normal_buff = ogle_gen_buffer(s,size,ResourceStorageModeShared);
                ogle_buffer_data_named(s,size,&mesh_r.normal_buff,(void*)mesh->normal_data);
//                mesh_r.normal_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->normal_count,ResourceStorageModeShared,12,mesh->normal_count);
//                RenderGPUMemory::UploadBufferData(&mesh_r.normal_buff,(void*)mesh->normal_data,sizeof(float) * mesh->normal_count);
                is_valid++;
            }

            if(mesh->uv_count > 0)
            {
                u64 size = sizeof(float) * mesh->uv_count;
                mesh_r.uv_buff = ogle_gen_buffer(s,size,ResourceStorageModeShared);
                ogle_buffer_data_named(s,size,&mesh_r.uv_buff,(void*)mesh->uv_data);
//                mesh_r.uv_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->uv_count,ResourceStorageModeShared,8,mesh->uv_count);
//                RenderGPUMemory::UploadBufferData(&mesh_r.uv_buff,(void*)mesh->uv_data,sizeof(float) * mesh->uv_count);
                is_valid++;
            }

            if(mesh->uv2_count > 0)
            {
                u64 size = sizeof(float) * mesh->uv2_count;
                mesh_r.uv2_buff = ogle_gen_buffer(s,size,ResourceStorageModeShared);
                ogle_buffer_data_named(s,size,&mesh_r.uv2_buff,(void*)mesh->uv2_data);
//                mesh_r.uv2_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->uv2_count,ResourceStorageModeShared,8,mesh->uv2_count);
//                RenderGPUMemory::UploadBufferData(&mesh_r.uv2_buff,(void*)mesh->uv2_data,sizeof(float) * mesh->uv2_count);
                is_valid++;
            }
            
            if(mesh->index16_count > 0)
            {
                u64 size = sizeof(float) * mesh->index16_count;
                mesh_r.element_buff = ogle_gen_buffer(s,size,ResourceStorageModeShared);
                ogle_buffer_data_named(s,size,&mesh_r.element_buff,(void*)mesh->index_16_data);                
//                mesh_r.element_buff = RenderGPUMemory::NewBufferWithLength(sizeof(u16) * mesh->index16_count,ResourceStorageModeShared,4,mesh->index16_count);
//                RenderGPUMemory::UploadBufferData(&mesh_r.element_buff,(void*)mesh->index_16_data,sizeof(u16) * mesh->index16_count);
                mesh_r.element_buff.index_type = IndexTypeUInt16;
                is_valid++;
            }
            //NOTE(RAY):For now we require that you have met all the data criteria
            if(is_valid >= 4)
            {
//                GPUResourceCache::AddGPUResource(mesh,mesh_r);
                mesh->mesh_resource = mesh_r;
            }
            else
            {
                Assert(false);
            }
        }
    }

    void UploadModelAssetToGPUTest(ModelAsset* ma)
    {
        for(int i = 0;i < ma->meshes.count;++i)
        {
            int is_valid = 0;
            GPUMeshResource mesh_r;
            MeshAsset* mesh = (MeshAsset*)ma->meshes.base + i;
            if(mesh->vertex_count > 0)
            {
                mesh_r.vertex_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->vertex_count,ResourceStorageModeShared,12,mesh->vertex_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.vertex_buff,(void*)mesh->vertex_data,sizeof(float) * mesh->vertex_count);
                is_valid++;
            }

            if(mesh->normal_count > 0)
            {
                mesh_r.normal_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->normal_count,ResourceStorageModeShared,12,mesh->normal_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.normal_buff,(void*)mesh->normal_data,sizeof(float) * mesh->normal_count);
                is_valid++;
            }

            if(mesh->uv_count > 0)
            {
                mesh_r.uv_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->uv_count,ResourceStorageModeShared,8,mesh->uv_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.uv_buff,(void*)mesh->uv_data,sizeof(float) * mesh->uv_count);
                is_valid++;
            }

            if(mesh->uv2_count > 0)
            {
                mesh_r.uv2_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->uv2_count,ResourceStorageModeShared,8,mesh->uv2_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.uv2_buff,(void*)mesh->uv2_data,sizeof(float) * mesh->uv2_count);
                is_valid++;
            }
            
            if(mesh->index16_count > 0)
            {
                mesh_r.element_buff = RenderGPUMemory::NewBufferWithLength(sizeof(u16) * mesh->index16_count,ResourceStorageModeShared,4,mesh->index16_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.element_buff,(void*)mesh->index_16_data,sizeof(u16) * mesh->index16_count);
                mesh_r.element_buff.index_type = IndexTypeUInt16;
                is_valid++;
            }
            //NOTE(RAY):For now we require that you have met all the data criteria
            if(is_valid >= 4)
            {
//                GPUResourceCache::AddGPUResource(mesh,mesh_r);
                mesh->mesh_resource = mesh_r;
            }
            else
            {
                Assert(false);
            }
        }
    }

    void UploadModelAssetToGPU(ModelAsset* ma)
    {
        for(int i = 0;i < ma->meshes.count;++i)
        {
            int is_valid = 0;
            GPUMeshResource mesh_r;
            MeshAsset* mesh = (MeshAsset*)ma->meshes.base + i;
            if(mesh->vertices.set_count > 0)
            {
                mesh_r.vertex_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->vertices.set_count,ResourceStorageModeShared,12,mesh->vertices.set_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.vertex_buff,(void*)mesh->vertices.base,sizeof(f32) * mesh->vertices.set_count);
                is_valid++;
            }
            if(mesh->normals.set_count > 0)
            {
                mesh_r.normal_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->normals.set_count,ResourceStorageModeShared,12,mesh->normals.set_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.normal_buff,(void*)mesh->normals.base,sizeof(float) * mesh->normals.set_count);
                is_valid++;
            }
            if(mesh->uvs.set_count > 0)
            {
                mesh_r.uv_buff = RenderGPUMemory::NewBufferWithLength(sizeof(float) * mesh->uvs.set_count,ResourceStorageModeShared,8,mesh->uvs.set_count);
                RenderGPUMemory::UploadBufferData(&mesh_r.uv_buff,(void*)mesh->uvs.base,sizeof(float) * mesh->uvs.set_count);
                is_valid++;
            }
            if(mesh->elements.count > 0)
            {
                mesh_r.element_buff = RenderGPUMemory::NewBufferWithLength(sizeof(uint32_t) * mesh->elements.count,ResourceStorageModeShared,4,mesh->elements.count);
                RenderGPUMemory::UploadBufferData(&mesh_r.element_buff,(void*)mesh->elements.base,sizeof(u32) * mesh->elements.count);
                mesh_r.element_buff.index_type = IndexTypeUInt32;
                is_valid++;
            }
            if(is_valid == 4)
                GPUResourceCache::AddGPUResource(mesh,mesh_r);
        }
    }

    void GetMeshCount(FbxNode* node, int* count)
    {
        for (int i = 0; i < node->GetChildCount(); ++i)
        {
            FbxNode* at_node = node->GetChild(i);
            FbxNodeAttribute* attribute = at_node->GetNodeAttribute();
            if (attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                (*count)++;
            }
            GetMeshCount(at_node, count);
        }
    }

    void GetMeshes(FbxNode* node, ModelAsset* model)
    {
        int child_count = node->GetChildCount(); 
        for (int node_i = 0; node_i < child_count; ++node_i)
        {
            FbxNode* at_node = node->GetChild(node_i);
            FbxNodeAttribute* attribute = at_node->GetNodeAttribute();
            if (attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                FbxNode* current_node = at_node;
                int lMaterialIndex;
                FbxProperty lProperty;
                int lNbMat = at_node->GetSrcObjectCount<FbxSurfaceMaterial>();
                int lMaterialCount = 0;
                {
                        lMaterialCount = at_node->GetMaterialCount();    
                }

                int lMtrlCount = 0;
                FbxMesh* pMesh = at_node->GetMesh();
                for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
                {
                    FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial( l);
                    if (leMat)
                    {
                        int lMaterialCount = 0;
                        if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect)
                        {
                            lMaterialCount = lMtrlCount;
                            int a = 0;
                        }
                        if(leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                        {
                            int a = 0;
                        }
                        if(leMat->GetMappingMode() == FbxGeometryElement::eAllSame)
                        {
                            int a = 0;
                        }
                        if(leMat->GetMappingMode() == FbxGeometryElement::eByPolygon)
                        {
                            int a = 0;
                        }
                        if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex)
                        {
#if 0
                            int i;
                            lString = "           Indices: ";
                            int lIndexArrayCount = leMat->GetIndexArray().GetCount();
                            for (i = 0; i < lIndexArrayCount; i++)
                            {
                                lString += leMat->GetIndexArray().GetAt(i);
                                if (i < lIndexArrayCount - 1)
                                {
                                    lString += ", ";
                                }
                            }
                            lString += "\n";
#endif
                        }
                    }
                }

                for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++){
                    FbxSurfaceMaterial *lMaterial = current_node->GetSrcObject<FbxSurfaceMaterial>(lMaterialIndex);
                    bool lDisplayHeader = true;
                    //go through all the possible textures
                    if(lMaterial)
                    {
                        for(int lLayerIndex = 0;lLayerIndex < FbxLayerElement::sTypeTextureCount; lLayerIndex++)
                        {
                            lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lLayerIndex]);
                            //FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex);
                        }
                    }//end if(lMaterial)
                }// end for lMaterialIndex
                
                Yostr node_name;
                YoyoVector vertex_vector;
                YoyoVector element_vector;
                YoyoVector normal_vector;
                YoyoVector uv_vector;
                YoyoVector tangent_vector;
                YoyoVector bitangent_vector;
            
                FbxNodeAttribute::EType attribute_type = current_node->GetNodeAttribute()->GetAttributeType();
                switch (attribute_type)
                {
                    case FbxNodeAttribute::eMesh:
                    {
                        PlatformOutput(asset_system_log, "MeshNAME:: %s \n", current_node->GetName());
                        const char* s = current_node->GetName();
                        node_name = CreateStringFromLiteral((char*)s,&StringsHandler::string_memory);
                
                        FbxMesh* mesh = current_node->GetMesh();
                        //TODO(Ray):Later log any non triangular meshes for now we need to know.
                        if (!mesh->IsTriangleMesh())
                        {
                            PlatformOutput(true,"Mesh is not triangulated :: %s \n", model->model_name.String);
                            //TODO(ray):Log this and output error.
                            Yostr error_string = AppendString(CreateStringFromLiteral("NonTriangulatedMesh :: ", &StringsHandler::transient_string_memory), model->model_name, &StringsHandler::transient_string_memory);
                            error_strings[error_count++] = error_string;
                            continue;
                        }

                        Assert(mesh->IsTriangleMesh());
                        int lPolygonCount = mesh->GetPolygonCount();
                        FbxVector4* lControlPoints = mesh->GetControlPoints();
                        int* polygon_vertices = mesh->GetPolygonVertices();
                        int pv_count = mesh->GetPolygonVertexCount();
                        //NOTE(Ray):Should be ok if we are a triangle mesh
                        //but we need to verify that every polygon is 3 sided.
                        
                        int lPolygonSize = mesh->GetPolygonSize(node_i);
                        if(lPolygonSize == -1)continue;//out of bounds
                        Assert(lPolygonSize == 3);
                        uint float_count = lPolygonCount * lPolygonSize * 3;
                        uint t_b_float_count = lPolygonCount * lPolygonSize * 4;
                        uint uv_count = lPolygonCount * lPolygonSize * 2;
                        uint element_count = lPolygonCount * lPolygonSize;
                        uint32_t float_size = sizeof(float);
                        uint32_t int_size = sizeof(uint32_t);

#if 1
                        uint32_t alignment = 0;
                        vertex_vector = YoyoInitVectorWithAligment(float_count, sizeof(float),false,alignment);
                        element_vector = YoyoInitVectorWithAligment(element_count, sizeof(uint32_t),false,alignment);
                        normal_vector = YoyoInitVectorWithAligment(float_count, sizeof(float),false,alignment);
                        uv_vector = YoyoInitVectorWithAligment(uv_count, sizeof(float),false,alignment);
                        tangent_vector = YoyoInitVectorWithAligment(t_b_float_count, sizeof(float),false,alignment);
                        bitangent_vector = YoyoInitVectorWithAligment(t_b_float_count, sizeof(float),false,alignment);
#else
                        vertex_vector = YoyoInitVector(float_count, float,false);
                        element_vector = YoyoInitVector(element_count, uint32_t,false);
                        normal_vector = YoyoInitVector(float_count, float,false);
                        uv_vector = YoyoInitVector(uv_count, float,false);
                        tangent_vector = YoyoInitVector(t_b_float_count, float,false);
                        bitangent_vector = YoyoInitVector(t_b_float_count, float,false);
#endif

                        int vertexId = 0;
                        for (int pi = 0; pi < lPolygonCount; ++pi)
                        {
                            PlatformOutput(asset_system_log, "Start Poly ---------------\n");
                            for (int vi = 0; vi < lPolygonSize; vi++)
                            {
                                u32 element_index = mesh->GetPolygonVertex(pi, vi);
                                u32 element_index_raw = mesh->GetPolygonVertexIndex(pi) + vi;
                                u32 final_element_index = element_index_raw;

                                FbxVector4 v = mesh->GetControlPointAt(element_index);
                                float3data vertex = { (float)v[0], (float)v[1], (float)v[2] };

                                YoyoPushBack(&element_vector, final_element_index);

                                int v_index = final_element_index * 3;
                                YoyoSetVectorElement(&vertex_vector, v_index, &vertex.i[0]);
                                YoyoSetVectorElement(&vertex_vector, v_index + 1, &vertex.i[1]);
                                YoyoSetVectorElement(&vertex_vector, v_index + 2, &vertex.i[2]);
                        
                                YoyoSetVectorElement(&vertex_vector, v_index, &vertex.i[0]);
                                YoyoSetVectorElement(&vertex_vector, v_index + 1, &vertex.i[1]);
                                YoyoSetVectorElement(&vertex_vector, v_index + 2, &vertex.i[2]);

                                PlatformOutput(asset_system_log, "Element :: %d Vertex:: x::%f y::%f z::%f  \n", final_element_index, vertex.x, vertex.y, vertex.z);
                                int normal_count = mesh->GetElementNormalCount();
                                for (int l = 0; l < normal_count; ++l)
                                {
                                    Assert(mesh->GetElementNormalCount() == 1);
                                    FbxGeometryElementNormal* leNormal = mesh->GetElementNormal(l);
                                    //FBXSDK_sprintf(header, 100, "            Normal: ");
                                    if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                                    {
                                        switch (leNormal->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(vertexId);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = vertexId * 3;
                                                YoyoSetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                YoyoSetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                YoyoSetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                //PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leNormal->GetIndexArray().GetAt(vertexId);
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(id);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = vertexId * 3;
                                                YoyoSetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                YoyoSetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                YoyoSetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                //PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            default:
                                            {
                                                Assert(false);
                                            }
                                            break; // other reference modes not shown here!
                                        }
                                    }
                                    else if (leNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                                    {
                                        switch (leNormal->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(final_element_index);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = final_element_index * 3;
                                                YoyoSetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                YoyoSetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                YoyoSetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                //PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leNormal->GetIndexArray().GetAt(final_element_index);
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(id);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = final_element_index * 3;
                                                YoyoSetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                YoyoSetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                YoyoSetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                PlatformOutput(asset_system_log, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            default:
                                            {
                                                Assert(false);
                                            }
                                            break;
                                        }
                                    }
                                }
                                //TODO(Ray):Look at the case of more than 1 uv
                                //for now we are only allowing one uv set per mesh
                                int uv_count = mesh->GetElementUVCount();
                                if(uv_count == 0)Assert(false);//NOTE(Ray):We are saying you must have uvs for now.
                                if(uv_count > 1)uv_count = 1;
                                for (int l = 0; l < uv_count; ++l)
                                {
                                    FbxGeometryElementUV* leUV = mesh->GetElementUV(l);
                                    switch (leUV->GetMappingMode())
                                    {
                                        case FbxGeometryElement::eByControlPoint:
                                            switch (leUV->GetReferenceMode())
                                            {
                                                case FbxGeometryElement::eDirect:
                                                {
                                                    //TODO(Ray):Need to check these import methods
                                                    Assert(false);
                                                    FbxVector2 f_bv2 = leUV->GetDirectArray().GetAt(final_element_index);
                                                    float2data uv = {(float)f_bv2[0], (float)f_bv2[1]};
                                                    int uv_index = final_element_index * 2;
                                                    YoyoSetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    YoyoSetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
                                                    //PlatformOutput(print_fbx, " UV:: x::%f y::%f  \n", uv.x(), uv.y());
                                                }break;
                                                case FbxGeometryElement::eIndexToDirect:
                                                {
                                                    //TODO(Ray):Need to check these import methods
                                                    Assert(false);
                                                    int id = leUV->GetIndexArray().GetAt(final_element_index);
                                                    FbxVector2 f_bv2 = leUV->GetDirectArray().GetAt(id);
                                                    float2data uv = {(float)f_bv2[0], (float)f_bv2[1]};
                                                    int uv_index = final_element_index * 2;
                                                    YoyoSetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    YoyoSetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
                                                    //PlatformOutput(print_fbx, " UV:: x::%f y::%f  \n", uv.x(), uv.y());
                                                }
                                                break;
                                                default:
                                                    break; // other reference modes not shown here!
                                            }
                                            break;
                                        case FbxGeometryElement::eByPolygonVertex:
                                        {
                                            int lTextureUVIndex = mesh->GetTextureUVIndex(pi, vi);
                                            switch (leUV->GetReferenceMode())
                                            {
                                                case FbxGeometryElement::eDirect:
                                                case FbxGeometryElement::eIndexToDirect:
                                                {
                                                    FbxVector2 f_bv2 = leUV->GetDirectArray().GetAt(lTextureUVIndex);
                                                    float2data uv = {(float)f_bv2[0], (float)f_bv2[1]};
                                                    int uv_index = final_element_index * 2;
                                                    YoyoSetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    YoyoSetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
                                                    //PlatformOutput(print_fbx, " UV:: x::%f y::%f  \n", uv.x(), uv.y());
                                                    //Display2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
                                                }
                                                break;
                                                default:
                                                    break; // other reference modes not shown here!
                                            }
                                        }
                                        break;
                                        case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
                                        case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
                                        case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
                                            break;
                                        default:
                                            break;
                                    }
                                }

                                for (int l = 0; l < mesh->GetElementTangentCount(); ++l)
                                {
                                    Assert(mesh->GetElementTangentCount() == 1);
                                    FbxGeometryElementTangent* leTangent = mesh->GetElementTangent(l);

                                    if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                                    {
                                        switch (leTangent->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(vertexId);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = vertexId * 4;
                                                YoyoSetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                YoyoSetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                YoyoSetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                YoyoSetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
                                                //PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leTangent->GetIndexArray().GetAt(vertexId);
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(id);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = vertexId * 4;
                                                YoyoSetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                YoyoSetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                YoyoSetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                YoyoSetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
                                                PlatformOutput(asset_system_log, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            default:
                                                break; // other reference modes not shown here!
                                        }
                                    }
                                    else if (leTangent->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                                    {
                                        switch (leTangent->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(final_element_index);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = final_element_index * 4;
                                                YoyoSetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                YoyoSetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                YoyoSetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                YoyoSetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
///									PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leTangent->GetIndexArray().GetAt(final_element_index);
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(id);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = final_element_index * 4;
                                                YoyoSetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                YoyoSetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                YoyoSetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                YoyoSetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
//									PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            default:
                                                break; // other reference modes not shown here!
                                        }
                                    }
                                }

                                for (int l = 0; l < mesh->GetElementBinormalCount(); ++l)
                                {
                                    Assert(mesh->GetElementBinormalCount() == 1);
                                    FbxGeometryElementBinormal* leBinormal = mesh->GetElementBinormal(l);
                                    if (leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
                                    {
                                        switch (leBinormal->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 f_bitangent = leBinormal->GetDirectArray().GetAt(vertexId);
                                                float4data bitangent = {(float)f_bitangent[0], (float)f_bitangent[1], (float)f_bitangent[2], (float)f_bitangent[3]};
                                                int index = vertexId * 4;
                                                YoyoSetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
//									PlatformOutput(print_fbx, " BITangent:: x::%f y::%f z::%f  \n", f_bitangent[0], f_bitangent[1], f_bitangent[2]);
                                            }
                                            //          Display3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
                                            break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leBinormal->GetIndexArray().GetAt(vertexId);
                                                FbxVector4 f_bitangent = leBinormal->GetDirectArray().GetAt(id);
                                                float4data bitangent = {(float)f_bitangent[0], (float)f_bitangent[1], (float)f_bitangent[2], (float)f_bitangent[3]};
                                                int index = vertexId * 4;
                                                YoyoSetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
//									PlatformOutput(print_fbx, " BITangent:: x::%f y::%f z::%f 2::%f  \n", f_bitangent[0], f_bitangent[1], f_bitangent[2], f_bitangent[3]);
                                                //        Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));
                                            }
                                            break;
                                            default:
                                                break; // other reference modes not shown here!
                                        }
                                    }
                                    else if (leBinormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
                                    {
                                        switch (leBinormal->GetReferenceMode())
                                        {
                                            case FbxGeometryElement::eDirect:
                                            {
                                                FbxVector4 f_bitangent = leBinormal->GetDirectArray().GetAt(final_element_index);
                                                float4data bitangent = {(float)f_bitangent[0], (float)f_bitangent[1], (float)f_bitangent[2], (float)f_bitangent[3]};
                                                int index = final_element_index * 4;
                                                YoyoSetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
//									PlatformOutput(print_fbx, " BITangent:: x::%f y::%f z::%f w::%f \n", f_bitangent[0], f_bitangent[1], f_bitangent[2], f_bitangent[3]);
                                            }
                                            //          Display3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
                                            break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leBinormal->GetIndexArray().GetAt(final_element_index);
                                                FbxVector4 f_bitangent = leBinormal->GetDirectArray().GetAt(id);
                                                float4data bitangent = {(float)f_bitangent[0], (float)f_bitangent[1], (float)f_bitangent[2], (float)f_bitangent[3]};
                                                int index = final_element_index * 4;
                                                YoyoSetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                YoyoSetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
//									PlatformOutput(print_fbx, " BITangent:: x::%f y::%f z::%f w::%f \n", f_bitangent[0], f_bitangent[1], f_bitangent[2], f_bitangent[3]);
                                                //        Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));
                                            }
                                            break;
                                            default:
                                                break; // other reference modes not shown here!
                                        }
                                    }
                                }
                                vertexId++;
                            }

                            PlatformOutput(asset_system_log, "End Poly ---------------\n");
                        }
                    }break;
                    default:
                    {
                        Assert(false);
                    }
                    break;
                }
            
                MeshAsset mesh;
                mesh.name = node_name;
                mesh.vertices = vertex_vector;
                mesh.elements = element_vector;
                mesh.normals = normal_vector;
                mesh.tangents = tangent_vector;
                mesh.bi_tangents = bitangent_vector;
                mesh.uvs = uv_vector;

                YoyoPushBack(&model->meshes, mesh);

                YoyoClearVector(&vertex_vector);
                YoyoClearVector(&element_vector);
                YoyoClearVector(&normal_vector);
                YoyoClearVector(&uv_vector);
                YoyoClearVector(&tangent_vector);
                YoyoClearVector(&bitangent_vector);
            }
            GetMeshes(at_node, model);
        }
    }

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(fbx_manager->GetIOSettings()))
#endif
    
    bool LoadScene(const char* pFilename, ModelAsset* model)
    {
        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        char lPassword[1024];

        // Get the file version number generate by the FBX SDK.
        FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(fbx_manager, "");

        // Initialize the importer by providing a filename.
        const bool lImportStatus = lImporter->Initialize(pFilename, -1, fbx_manager->GetIOSettings());
        lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

        if (!lImportStatus)
        {
            FbxString error = lImporter->GetStatus().GetErrorString();
            PlatformOutput(asset_system_log, "Call to FbxImporter::Initialize() failed.\n");
            PlatformOutput(asset_system_log, "Error returned: %s\n\n", error.Buffer());

            if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
            {
                PlatformOutput(asset_system_log, "FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
                PlatformOutput(asset_system_log, "FBX file format version for file '%s' is %d.%d.%d\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
            }
            return false;
        }

        if (lImporter->IsFBX())
        {
            PlatformOutput(asset_system_log, "FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
            IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
            IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
            IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
            IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
            IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
            IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
        }

        // Import the scene.
        bool lStatus = lImporter->Import(fbx_scene);

        if (!lStatus && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            PlatformOutput(asset_system_log, "Please enter password: ");
            lPassword[0] = '\0';
            FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
                scanf("%s", lPassword);
            FBXSDK_CRT_SECURE_NO_WARNING_END
                FbxString lString(lPassword);
            IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
            IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
            lStatus = lImporter->Import(fbx_scene);
            if (!lStatus && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
            {
                PlatformOutput(asset_system_log, "\nPassword is wrong, import aborted.\n");
            }
        }

        FbxScene* lScene = FbxScene::Create(fbx_manager, "myScene");
        // Import the contents of the file into the scene.
        lImporter->Import(lScene);
        
        FbxCollection* collection = FbxCollection::Create(fbx_manager, "mycollection");
        
        FbxNode* root = lScene->GetRootNode();
        
        int mesh_count_in_scene = 0;
        GetMeshCount(root, &mesh_count_in_scene);
        if(mesh_count_in_scene > 0)
        {
            model->meshes = YoyoInitVector(mesh_count_in_scene, MeshAsset,false);
            GetMeshes(root, model);
        }
        lImporter->Destroy();
        return lStatus;
    }

    bool GLTFLoadModel(const char* file_path,ModelAsset* result)
    {
        PlatformOutput(asset_system_log, "TestLoading with CGLTF");
        bool is_success = false;
        result->model_name = CreateStringFromLiteral(file_path, &StringsHandler::transient_string_memory);

        cgltf_options options = {};
        cgltf_data* data = NULL;
        cgltf_result aresult = cgltf_parse_file(&options,file_path, &data);
        if (aresult == cgltf_result_success)
        {
#if 0
                for(int i = 0;i < data->materials_count;++i)
                {
                    cgltf_material mat = data->materials[i];
                    int a = 0;
                }
#endif
                
            /* TODO make awesome stuff */
            for(int i = 0;i < data->buffers_count;++i)
            {
                
                cgltf_result rs = cgltf_load_buffers(&options, data, data->buffers[i].uri);
                InProgressMetaFile mf = {};
                if(data->meshes_count > 0)
                {
                    MetaFiles::StartMetaFileCreation(&mf,CreateStringFromLiteral(file_path,&StringsHandler::transient_string_memory),data->meshes_count);
                }

                for(int i = 0;i < data->meshes_count;++i)
                {
                    cgltf_mesh mes = data->meshes[i];
                    PlatformOutput(true, mes.name);                
                    MetaFiles::AddMeshToMetaFile(&mf,mes);
                }
                *result = mf.model;
                MetaFiles::EndMetaFileCreation(&mf);
                is_success = true;
            }
//            cgltf_free(data);
        }
        return is_success;        
    }
    
    bool FBXSDKLoadModel(char* file_path,ModelAsset* result)
    {
        PlatformOutput(asset_system_log, "TestLoading with FBXSDK");
        //ModelAsset result = {};
        bool is_success = false;
        result->model_name = CreateStringFromLiteral(file_path, &StringsHandler::transient_string_memory);
        is_success = LoadScene(file_path, result);
        return is_success;
    }
    
    //TODO(Ray):Next we need a facility for handling assets that are shared in memory between
    //mutiple assets.
    //The most obvious solution is a reference counting solution.
    //When an asset is done with another asset it can "release" it and decrement the internally handled
    //counter.
    //Until the counter reaches zero which at that point we really will "release"
    //Seems ok but there are also issues with it.

//NOTE(Ray):Maybe or we will just release everything on scence change for starters.
    //TODO(Ray):Handle loosing the device reference and rebuild everthing in that case...
    
    bool AddOrGetTexture(Yostr path,LoadedTexture* result,GLTexture* gl_tex_result)
    {
        bool bool_result = false;
        //result = nullptr;
        //TODO(Ray):Propery handling of a unfound asset
        //if(!TextureCache::DoesTextureExist(&path))
        uint64_t t_key = StringsHandler::StringHash(path.String,path.Length) % ULONG_MAX;
        if(!AnythingCacheCode::DoesThingExist(&texture_cache,&t_key))
        {
            //TODO(Ray):This gets it from disk but we should be able to get it from anywhere .. network stream etc...
            //We will add a facility for tagging assets and retrieving base on criteria and only the asset retriever
            //(Serializer) like system will be the only one to deal with what we are getting it from.
           // LoadedTexture tex = Resource::GetLoadedImage(path.String);
            LoadedTexture tex = Resource::GetLoadedImage(path.String, 4);
            if(tex.texels)
            {
                TextureDescriptor td = RendererCode::Texture2DDescriptorWithPixelFormat(PixelFormatRGBA8Unorm,tex.dim.x(),tex.dim.y(),false);
                td.storageMode = StorageModeManaged;
                tex.texture = RendererCode::NewTextureWithDescriptor(td);
                RenderRegion region = {};
                region.origin = float3(0.0f);
                region.size = float2(tex.dim.x(), tex.dim.y());
                
                //TODO(Ray):Handle the texture descritptor memory?
                RenderGPUMemory::ReplaceRegion(tex.texture,region,0,tex.texels,tex.bytes_per_pixel * tex.dim.x());

//                tex.texture.state = PlatformGraphicsAPI_Metal::GPUAllocateTexture(tex.texels,tex.bytes_per_pixel,tex.dim.x(),tex.dim.y());
                AnythingCacheCode::AddThing(&texture_cache,&t_key,&tex);
                *result = tex;


                *gl_tex_result = ogle_tex_image_2d(&DefferedRenderer::ogl_test_state,tex.texels,tex.dim,PixelFormatRGBA8Unorm,TextureUsageShaderRead);
                AnythingCacheCode::AddThing(&gl_texture_cache,&t_key,gl_tex_result);
                bool_result = true;
            }
        }
        else
        {
            GLTexture*gl_tex = (GLTexture*)AnythingCacheCode::GetThing(&gl_texture_cache,&t_key);
            if(gl_tex)
            {
                *gl_tex_result = *gl_tex;
                bool_result = true;
            }
            
            LoadedTexture*tex = (LoadedTexture*)AnythingCacheCode::GetThing(&texture_cache,&t_key);
            if(tex)
            {
                *result = *tex;
                bool_result = true;
            }

        }
        return bool_result;
    }

#if 0
    bool AddOrGetTextureFromMemory(void* data,uint64_t size,LoadedTexture* result)
    {
        bool bool_result = false;
        //result = nullptr;
        //TODO(Ray):Propery handling of a unfound asset
        uint64_t t_key = YoyoMeowHashFunction(data,size);//StringsHandler::StringHash(path.String,path.Length);
        if(!AnythingCacheCode::DoesThingExist(&texture_cache,&t_key))
        {
            //TODO(Ray):This gets it from disk but we should be able to get it from anywhere .. network stream etc...
            //We will add a facility for tagging assets and retrieving base on criteria and only the asset retriever
            //(Serializer) like system will be the only one to deal with what we are getting it from.
           // LoadedTexture tex = Resource::GetLoadedImage(path.String);
            LoadedTexture tex = {};
            Resource::GetImageFromMemory(data,size,&tex,4);
            if(tex.texels)
            {
                tex.texture.state = PlatformGraphicsAPI_Metal::GPUAllocateTexture(tex.texels,tex.bytes_per_pixel,tex.dim.x(),tex.dim.y());
                AnythingCacheCode::AddThing(&texture_cache,&t_key,&tex);
                *result = tex;
                bool_result = true;
            }
        }
        else
        {
            LoadedTexture*tex = (LoadedTexture*)AnythingCacheCode::GetThing(&texture_cache,&t_key);
            if(tex)
            {
                *result = *tex;
                bool_result = true;
            }
        }
        return bool_result;
    }
#endif
    
#if 1
    //Take in a json doc here
    RenderMaterial CreateMaterialFromDescription(Yostr* vs_name,Yostr* as_name,float4 base_color)
    {
        //test material
        RenderMaterial mat_result = {};
        mat_result.type = -1;//default opaque
//        float4 base_color = float4(1);
        mat_result.inputs.base_color = base_color;
        
        RenderShader shader;
        RenderShaderCode::InitShaderFromDefaultLib(&shader,vs_name->String,as_name->String);
        ShaderTextureSlot slot;
        slot.material_resource_id = 0;
        shader.texture_slot_count = 1;
        shader.texture_slots[0] = slot;
        
        RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,1);
        //render_pipeline_descriptor.label = "test";
        render_pipeline_descriptor.vertex_function = shader.vs_object;
        render_pipeline_descriptor.fragment_function = shader.ps_object;
        render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        mat_result.shader = shader;
        
        //Vertex descriptions and application
        //TODO(Ray):So now we are going to return a vertex descriptor of our own and handle it
        //than set it back and let teh api do the translation for us. Thats the pattern here for pipeline
        //state creation... than we will do serialization to the pipeline desc for
        //pre creation of our pipeline states to save on load times and perfs if need be. but can save that
        //for much later stages.
        
        //TODO(Ray):Introspect the shader and build this automagically from the introspected shader
        //source.  We can get the datatypes inputs to the vertex function from the shader.
        default_vertex_descriptor = RenderEncoderCode::NewVertexDescriptor();
        VertexAttributeDescriptor vad;
        vad.format = VertexFormatFloat3;
        vad.offset = 0;
        vad.buffer_index = 0;
        VertexAttributeDescriptor n_ad;
        n_ad.format = VertexFormatFloat3;
        n_ad.offset = 0;
        n_ad.buffer_index = 1;
        VertexAttributeDescriptor uv_ad;
        uv_ad.format = VertexFormatFloat2;
        uv_ad.offset = 0;
        uv_ad.buffer_index = 2;
        VertexBufferLayoutDescriptor vbld;
        vbld.step_function = step_function_per_vertex;
        vbld.step_rate = 1;
        vbld.stride = 12;
        VertexBufferLayoutDescriptor n_bld;
        n_bld.step_function = step_function_per_vertex;
        n_bld.step_rate = 1;
        n_bld.stride = 12;
        VertexBufferLayoutDescriptor uv_bld;
        uv_bld.step_function = step_function_per_vertex;
        uv_bld.step_rate = 1;
        uv_bld.stride = 8;
        RenderEncoderCode::AddVertexDescription(&default_vertex_descriptor,vad,vbld);
        RenderEncoderCode::AddVertexDescription(&default_vertex_descriptor,n_ad,n_bld);
        RenderEncoderCode::AddVertexDescription(&default_vertex_descriptor,uv_ad,uv_bld);
        RenderEncoderCode::SetVertexDescriptor(&render_pipeline_descriptor,&default_vertex_descriptor);
        
        RenderPipelineColorAttachmentDescriptorArray rpcada = render_pipeline_descriptor.color_attachments;
        RenderPipelineColorAttachmentDescriptor rad = rpcada.i[0];
        rad.pixelFormat = PixelFormatBGRA8Unorm;
        render_pipeline_descriptor.color_attachments.i[0] = rad;
        
        /*
         if(mat_result.type == 1)//transparent set blending
         {
         RenderPipelineColorAttachmentDescriptorArray rpcada = render_pipeline_descriptor.color_attachments;
         RenderPipelineColorAttachmentDescriptor rad = rpcada.i[0];
         rad.writeMask = ColorWriteMaskAll;
         rad.blendingEnabled = true;
         rad.destinationRGBBlendFactor = BlendFactorOneMinusSourceAlpha;
         rad.destinationAlphaBlendFactor = BlendFactorOne;
         rad.sourceRGBBlendFactor = BlendFactorSourceAlpha;
         rad.sourceAlphaBlendFactor = BlendFactorOne;
         render_pipeline_descriptor.color_attachments.i[0] = rad;
         }
         */
        
        //Create pipeline states
        RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
        mat_result.pipeline_state = pipeline_state;
        
        //create depth states
        DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
        depth_desc.depthWriteEnabled = true;
        depth_desc.depthCompareFunction = compare_func_less;
        DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
        mat_result.depth_stencil_state = depth_state;
        
        //            mat_result.texture_slots[mat_result.texture_count] = uploaded_texture;
        //            mat_result.texture_count++;
        //        test_material = mat_result;
        
        //NOTE(Ray):Probably move this to a more renderer specific area
        return mat_result;
    }
#endif
    
//Take in a json doc here
    RenderMaterial CreateDefaultMaterial()
    {
        //test material
        RenderMaterial mat_result = {};
        mat_result.type = -1;//default opaque
        float4 base_color = float4(1);
        mat_result.inputs.base_color = base_color;

        RenderShader shader;
        RenderShaderCode::InitShaderFromDefaultLib(&shader,"diffuse_vs","diffuse_color_fs");
        ShaderTextureSlot slot;
        slot.material_resource_id = 0;
        shader.texture_slot_count = 1;
        shader.texture_slots[0] = slot;

        RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,1);
//        render_pipeline_descriptor.label = "test";
        render_pipeline_descriptor.vertex_function = shader.vs_object;
        render_pipeline_descriptor.fragment_function = shader.ps_object;
        render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        mat_result.shader = shader;

        //Vertex descriptions and application
        //TODO(Ray):So now we are going to return a vertex descriptor of our own and handle it
        //than set it back and let teh api do the translation for us. Thats the pattern here for pipeline
        //state creation... than we will do serialization to the pipeline desc for
        //pre creation of our pipeline states to save on load times and perfs if need be. but can save that
        //for much later stages.

        //TODO(Ray):Introspect the shader and build this automagically from the introspected shader
        //source.  We can get the datatypes inputs to the vertex function from the shader.
        VertexDescriptor vertex_descriptor = RenderEncoderCode::NewVertexDescriptor();
        VertexAttributeDescriptor vad;
        vad.format = VertexFormatFloat3;
        vad.offset = 0;
        vad.buffer_index = 0;
        VertexAttributeDescriptor n_ad;
        n_ad.format = VertexFormatFloat3;
        n_ad.offset = 0;
        n_ad.buffer_index = 1;
        VertexAttributeDescriptor uv_ad;
        uv_ad.format = VertexFormatFloat2;
        uv_ad.offset = 0;
        uv_ad.buffer_index = 2;
        VertexBufferLayoutDescriptor vbld;
        vbld.step_function = step_function_per_vertex;
        vbld.step_rate = 1;
        vbld.stride = 12;
        VertexBufferLayoutDescriptor n_bld;
        n_bld.step_function = step_function_per_vertex;
        n_bld.step_rate = 1;
        n_bld.stride = 12;
        VertexBufferLayoutDescriptor uv_bld;
        uv_bld.step_function = step_function_per_vertex;
        uv_bld.step_rate = 1;
        uv_bld.stride = 8;
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,vad,vbld);
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,n_ad,n_bld);
        RenderEncoderCode::AddVertexDescription(&vertex_descriptor,uv_ad,uv_bld);
        RenderEncoderCode::SetVertexDescriptor(&render_pipeline_descriptor,&vertex_descriptor);

        RenderPipelineColorAttachmentDescriptorArray rpcada = render_pipeline_descriptor.color_attachments;
        RenderPipelineColorAttachmentDescriptor rad = rpcada.i[0];
        rad.pixelFormat = PixelFormatBGRA8Unorm;
        render_pipeline_descriptor.color_attachments.i[0] = rad;

//Create pipeline states    
        PipelineOption options = (PipelineOption)(PipelineOptionArgumentInfo | PipelineOptionBufferTypeInfo);
        RenderPipelineReflection ref_ptr;
        RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
        mat_result.pipeline_state = pipeline_state;

//After we create the pipeline state we can now get the arguments from the shaders and match them with the material
//The shader must have a slot for the material will match what it can        
//create depth states
        DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
        depth_desc.depthWriteEnabled = true;
        depth_desc.depthCompareFunction = compare_func_less;
        DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
        mat_result.depth_stencil_state = depth_state;
        return mat_result;
    }
    
//Take in a json doc here and shaders infer the vertex attributews from those.
    //also all inputs and types need to take in here and pass down to rendering api about
    //our data.
    RenderMaterial CreateDefaultQuadMaterial()
    {
        //test material
        RenderMaterial mat_result = {};
        mat_result.type = -1;//default opaque
        float4 base_color = float4(1);
        mat_result.inputs.base_color = base_color;

        RenderShader shader;
        RenderShaderCode::InitShaderFromDefaultLib(&shader,"composite_vs","composite_fs");
        ShaderTextureSlot slot;
        slot.material_resource_id = 0;
        shader.texture_slot_count = 1;
        shader.texture_slots[0] = slot;

        RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,0);
//        render_pipeline_descriptor.label = "test";
        render_pipeline_descriptor.vertex_function = shader.vs_object;
        render_pipeline_descriptor.fragment_function = shader.ps_object;
        render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
        mat_result.shader = shader;

        //Vertex descriptions and application
        //TODO(Ray):So now we are going to return a vertex descriptor of our own and handle it
        //than set it back and let teh api do the translation for us. Thats the pattern here for pipeline
        //state creation... than we will do serialization to the pipeline desc for
        //pre creation of our pipeline states to save on load times and perfs if need be. but can save that
        //for much later stages.

        //TODO(Ray):Introspect the shader and build this automagically from the introspected shader
        //source.  We can get the datatypes inputs to the vertex function from the shader.
        VertexDescriptor vertex_descriptor = RenderEncoderCode::NewVertexDescriptor();
        VertexAttributeDescriptor vad;
        vad.format = VertexFormatFloat3;
        vad.offset = 0;
        vad.buffer_index = 0;
        VertexAttributeDescriptor uv_ad;
        uv_ad.format = VertexFormatFloat2;
        uv_ad.offset = float3::size();
        uv_ad.buffer_index = 0;

        VertexBufferLayoutDescriptor vbld;
        vbld.step_function = step_function_per_vertex;
        vbld.step_rate = 1;
        vbld.stride = float3::size() + float2::size();
        
        RenderEncoderCode::AddVertexAttribute(&vertex_descriptor,vad);
        RenderEncoderCode::AddVertexAttribute(&vertex_descriptor,uv_ad);
        //RenderEncoderCode::AddVertexDescription(&vertex_descriptor,uv_ad,vbld);
        RenderEncoderCode::AddVertexLayout(&vertex_descriptor, vbld);//(&vertex_descriptor,uv_ad,uv_bld);
        RenderEncoderCode::SetVertexDescriptor(&render_pipeline_descriptor,&vertex_descriptor);

//Create pipeline states    
        RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
        Assert(pipeline_state.state);
        mat_result.pipeline_state = pipeline_state;

//create depth states            
        DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
        depth_desc.depthWriteEnabled = false;
        //depth_desc.depthCompareFunction = compare_func_less;
        DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
        mat_result.depth_stencil_state = depth_state;
            
//            mat_result.texture_slots[mat_result.texture_count] = uploaded_texture;
//            mat_result.texture_count++;                   
//        test_material = mat_result;

        //NOTE(Ray):Probably move this to a more renderer specific area
        return mat_result;
    }
};

#endif
