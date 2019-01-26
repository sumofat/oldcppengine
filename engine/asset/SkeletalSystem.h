namespace SkeletalSystem
{
/*//NOTE(Ray):Placeholder
 *

//GetSkeletonFromFBX
#if 0

        int stack_count = pScene->GetSrcObjectCount<FbxAnimStack>();
        for (int i = 0; i < stack_count; ++i)
        {
            FbxAnimStack* lAnimStack = pScene->GetSrcObject<FbxAnimStack>(i);
            FbxString lOutputString = "Animation Stack Name: ";
            lOutputString += lAnimStack->GetName();
            lOutputString += "\n";
            FBXSDK_printf(lOutputString);

            //DisplayAnimation(lAnimStack, pScene->GetRootNode());
            int l;
            int nbAnimLayers = lAnimStack->GetMemberCount<FbxAnimLayer>();

            lOutputString = "   contains ";
            if (nbAnimLayers)
            {
                lOutputString += nbAnimLayers;
                lOutputString += " Animation Layer";
                if (nbAnimLayers > 1)
                    lOutputString += "s";
            }
            lOutputString += "\n\n";
            FBXSDK_printf(lOutputString);
            for (l = 0; l < nbAnimLayers; l++)
            {
                FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>(l);

                lOutputString = "AnimLayer ";
                lOutputString += l;
                lOutputString += "\n";
                FBXSDK_printf(lOutputString);
                FbxNode* root = lScene->GetRootNode();

                DisplayAnimation(lAnimLayer, root, false);
            }
        }
        
        int skeleton_count_in_scene = 0;
        bool found_root = false;
        GetSkeletonCount(lScene,lScene->GetRootNode(), &skeleton_count_in_scene);
        Skeleton skeleton{};
        skeleton.joints = YoyoInitVector(skeleton_count_in_scene, Joint,false);
        if (skeleton_count_in_scene > 0)
        {
            GetSkeleton(lScene,root, &skeleton, string_mem, skeleton_count_in_scene, 0);
        }
            
        int numStacks = pScene->GetSrcObjectCount(FbxCriteria::ObjectTypeStrict(FbxAnimStack::ClassId));
        for (int i = 0; i < numStacks; ++i)
        {
            FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(pScene->GetSrcObject(FbxCriteria::ObjectTypeStrict(FbxAnimStack::ClassId), i));
            int numAnimLayers = pAnimStack->GetMemberCount(FbxCriteria::ObjectTypeStrict(FbxAnimLayer::ClassId));
		
            for (size_t j = 0; j < numAnimLayers; j++)
            {
                FbxAnimLayer* anim_layer = FbxCast<FbxAnimLayer>(pAnimStack->GetMember(FbxCriteria::ObjectType(FbxAnimLayer::ClassId)));
                FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(0);
                if(skeleton_count_in_scene > 0)
                {
                    Joint* joint = YoyoGetVectorElement(Joint,&skeleton.joints,i);
                    FbxNode* joint_node = joint->node;
                    AnimationClip clip = GetAnimationForStack(pAnimStack,&skeleton,lScene);
                    AnimationKeyFrame* keyframe;
                    while(keyframe = YoyoIterateVector(&clip.keyframes,AnimationKeyFrame))
                    {
                        PlatformOutput(true, "time_stamp : %f\n", keyframe->time_stamp);
                        JointPose* jpose;
                        while(jpose = YoyoIterateVector(&keyframe->target,JointPose))
                        {
                            PlatformOutput(true, "jpose : %f\n", jpose->p.x());
                        }
                    }
                }
            }
        }
#endif


#if 0    
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

    void GetMeshes(FbxNode* node, ModelAsset* model, MemoryArena* string_mem,MemoryArena* perm_string_mem)
    {
        int child_count = node->GetChildCount(); 
        for (int node_i = 0; node_i < child_count; ++node_i)
        {
            FbxNode* at_node = node->GetChild(node_i);
            FbxNodeAttribute* attribute = at_node->GetNodeAttribute();
            if (attribute->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                vector vertex_vector;
                vector element_vector;
                vector normal_vector;
                vector uv_vector;
                vector tangent_vector;
                vector bitangent_vector;

                FbxNode* current_node = at_node;//at_node->GetChild(i);
                Yostr node_name;
            
                FbxNodeAttribute::EType attribute_type = current_node->GetNodeAttribute()->GetAttributeType();
                switch (attribute_type) {
                    case FbxNodeAttribute::eMesh:
                    {
                        PlatformOutput(print_fbx, "MeshNAME:: %s", current_node->GetName());
                        const char* s = current_node->GetName();
                        node_name = *CreateStringFromLiteral((char*)s,perm_string_mem);
                
                        FbxMesh* mesh = current_node->GetMesh();
                        //TODO(Ray):Later log any non triangular meshes for now we need to know.
                        if (!mesh->IsTriangleMesh())
                        {
                            PlatformOutput("Mesh is not triangulated :: %s \n", model->model_name->String);
                            //TODO(ray):Log this and output error.
                            Yostr* error_string = AppendString(*CreateStringFromLiteral("NonTriangulatedMesh :: ", string_mem), *model->model_name, string_mem);
                            error_strings[error_count++] = error_string;
                            return;
                        }
                        Assert(mesh->IsTriangleMesh());
                        int lPolygonCount = mesh->GetPolygonCount();

                        FbxVector4* lControlPoints = mesh->GetControlPoints();
                        int* polygon_vertices = mesh->GetPolygonVertices();

                        int pv_count = mesh->GetPolygonVertexCount();
                        //NOTE(Ray):Should be ok if we are a triangle mesh
                        //but we need to verify that every polygon is 3 sided.
                        //
                        int lPolygonSize = mesh->GetPolygonSize(node_i);
                        Assert(lPolygonSize == 3);
                        uint float_count = lPolygonCount * lPolygonSize * 3;
                        uint t_b_float_count = lPolygonCount * lPolygonSize * 4;
                        uint uv_count = lPolygonCount * lPolygonSize * 2;
                        uint element_count = lPolygonCount * lPolygonSize;
                        vertex_vector = CreateVector(float_count, sizeof(f32));
                        element_vector = CreateVector(element_count, sizeof(u32));
                        normal_vector = CreateVector(float_count, sizeof(f32));
                        uv_vector = CreateVector(uv_count, sizeof(f32));
                        tangent_vector = CreateVector(t_b_float_count, sizeof(f32));
                        bitangent_vector = CreateVector(t_b_float_count, sizeof(f32));

                        int vertexId = 0;
                        for (int pi = 0; pi < lPolygonCount; ++pi)
                        {

                            // FbxGeometryElementUV* element_uv = mesh->GetEle

                            PlatformOutput(print_fbx, "Start Poly ---------------\n");

                            for (int vi = 0; vi < lPolygonSize; vi++)
                            {
                                u32 element_index = mesh->GetPolygonVertex(pi, vi);
                                u32 element_index_raw = mesh->GetPolygonVertexIndex(pi) + vi;
                                //                            u32 element_index_old = (u32)polygon_vertices[vertexId];
                                //if(element_index != element_index_old)Assert(false);
                                u32 final_element_index = element_index_raw;

                                FbxVector4 v = mesh->GetControlPointAt(element_index);

                                float3data vertex = { (float)v[0], (float)v[1], (float)v[2] };
                        
#if 0
                                bool found = false;
                                u32* at_index;
                                while (at_index = IterateVector(&element_vector, u32))
                                {
                                    if (*at_index == element_index)
                                    {
                                        found = true;
                                    }
                                }
                                ResetVectorIterator(&element_vector);
                                if (found)
                                {
                                    //    final_element_index = j / 3;
                                }
#endif

                                PushVectorElement(&element_vector, &final_element_index);

                                int v_index = final_element_index * 3;
                                SetVectorElement(&vertex_vector, v_index, &vertex.i[0]);
                                SetVectorElement(&vertex_vector, v_index + 1, &vertex.i[1]);
                                SetVectorElement(&vertex_vector, v_index + 2, &vertex.i[2]);
                        
                                SetVectorElement(&vertex_vector, v_index, &vertex.i[0]);
                                SetVectorElement(&vertex_vector, v_index + 1, &vertex.i[1]);
                                SetVectorElement(&vertex_vector, v_index + 2, &vertex.i[2]);

                                //PlatformOutput(print_fbx, "Element :: %d Vertex:: x::%f y::%f z::%f  \n", final_element_index, vertex.x(), vertex.y(), vertex.z());
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
                                                SetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                SetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                SetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                //PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leNormal->GetIndexArray().GetAt(vertexId);
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(id);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = vertexId * 3;
                                                SetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                SetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                SetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
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
                                                SetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                SetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                SetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                //PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leNormal->GetIndexArray().GetAt(final_element_index);
                                                FbxVector4 normal = leNormal->GetDirectArray().GetAt(id);
                                                float3data normal_input = {(float)normal[0], (float)normal[1], (float)normal[2]};
                                                int index = final_element_index * 3;
                                                SetVectorElement(&normal_vector, index, &normal_input.i[0]);
                                                SetVectorElement(&normal_vector, index + 1, &normal_input.i[1]);
                                                SetVectorElement(&normal_vector, index + 2, &normal_input.i[2]);
                                                PlatformOutput(print_fbx, " Normal:: x::%f y::%f z::%f  \n", normal[0], normal[1], normal[2]);
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
                                                    SetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    SetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
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
                                                    SetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    SetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
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
                                                    SetVectorElement(&uv_vector, uv_index, &uv.i[0]);
                                                    SetVectorElement(&uv_vector, uv_index + 1, &uv.i[1]);
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
                                                SetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                SetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                SetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                SetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
                                                //PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leTangent->GetIndexArray().GetAt(vertexId);
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(id);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = vertexId * 4;
                                                SetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                SetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                SetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                SetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
                                                PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
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
                                                SetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                SetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                SetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                SetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
///									PlatformOutput(print_fbx, " Tangent:: x::%f y::%f z::%f w::%f  \n", f_tangent[0], f_tangent[1], f_tangent[2], f_tangent[3]);
                                            }break;
                                            case FbxGeometryElement::eIndexToDirect:
                                            {
                                                int id = leTangent->GetIndexArray().GetAt(final_element_index);
                                                FbxVector4 f_tangent = leTangent->GetDirectArray().GetAt(id);
                                                float4data tangent = {(float)f_tangent[0], (float)f_tangent[1], (float)f_tangent[2], (float)f_tangent[3]};
                                                int index = final_element_index * 4;
                                                SetVectorElement(&tangent_vector, index, &tangent.i[0]);
                                                SetVectorElement(&tangent_vector, index + 1, &tangent.i[1]);
                                                SetVectorElement(&tangent_vector, index + 2, &tangent.i[2]);
                                                SetVectorElement(&tangent_vector, index + 3, &tangent.i[3]);
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
                                                SetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                SetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                SetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                SetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
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
                                                SetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                SetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                SetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                SetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
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
                                                SetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                SetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                SetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                SetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
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
                                                SetVectorElement(&bitangent_vector, index, &bitangent.i[0]);
                                                SetVectorElement(&bitangent_vector, index + 1, &bitangent.i[1]);
                                                SetVectorElement(&bitangent_vector, index + 2, &bitangent.i[2]);
                                                SetVectorElement(&bitangent_vector, index + 3, &bitangent.i[3]);
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

                            PlatformOutput(print_fbx, "End Poly ---------------\n");
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

                PushVectorElement(&model->meshes, (void*)&mesh);
                model->mesh_count++;
                ClearVector(&vertex_vector);
                ClearVector(&element_vector);
                ClearVector(&normal_vector);
                ClearVector(&uv_vector);
                ClearVector(&tangent_vector);
                ClearVector(&bitangent_vector);
            }
            GetMeshes(at_node, model, string_mem,perm_string_mem);
        }
    }


    bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, ModelAsset* model, MemoryArena* string_mem,MemoryArena* perm_string_mem)
    {
        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        char lPassword[1024];

        // Get the file version number generate by the FBX SDK.
        FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(pManager, "");

        // Initialize the importer by providing a filename.
        const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
        lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

        if (!lImportStatus)
        {
            FbxString error = lImporter->GetStatus().GetErrorString();
            PlatformOutput(print_fbx, "Call to FbxImporter::Initialize() failed.\n");
            PlatformOutput(print_fbx, "Error returned: %s\n\n", error.Buffer());

            if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
            {
                PlatformOutput(print_fbx, "FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
                PlatformOutput(print_fbx, "FBX file format version for file '%s' is %d.%d.%d\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
            }
            return false;
        }

        if (lImporter->IsFBX())
        {
            PlatformOutput(print_fbx, "FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
            IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
            IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
            IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
            IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
            IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
            IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
        }

        // Import the scene.
        bool lStatus = lImporter->Import(pScene);

        if (!lStatus && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            PlatformOutput(print_fbx, "Please enter password: ");
            lPassword[0] = '\0';
            FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
                scanf("%s", lPassword);
            FBXSDK_CRT_SECURE_NO_WARNING_END
                FbxString lString(lPassword);
            IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
            IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
            lStatus = lImporter->Import(pScene);
            if (!lStatus && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
            {
                PlatformOutput(print_fbx, "\nPassword is wrong, import aborted.\n");
            }
        }

        FbxScene* lScene = FbxScene::Create(pManager, "myScene");
        // Import the contents of the file into the scene.
        lImporter->Import(lScene);

        FbxNode* root = lScene->GetRootNode();
        int mesh_count_in_scene = 0;
        GetMeshCount(root, &mesh_count_in_scene);
        if(mesh_count_in_scene > 0)
        {
            model->meshes = YoyoInitVector(mesh_count_in_scene, sizeof(MeshAsset),false);
            GetMeshes(root, model, string_mem,perm_string_mem);
        }
        lImporter->Destroy();
        return lStatus;
    }

    ModelAsset FBXSDKLoadModel(char* file_path, FbxManager* lSdkManager,
                                       FbxScene* lScene, MemoryArena* string_mem_arena,MemoryArena* perm_string_mem)
    {
        PlatformOutput(print_fbx, "TestLoading with FBXSDK");
        ModelAsset result = {};
        bool is_success = false;
        result.model_name = CreateStringFromLiteral(file_path, string_mem_arena);
        is_success = LoadScene(lSdkManager, lScene, file_path, &result, string_mem_arena,perm_string_mem);
        return result;
    }
#endif
/*
    void LoadModelFromDisk(Yostr* path)
    {
        ModelAsset new_model_asset = FBXSDKLoadModel(path, fbx_manager, fbx_scene, &StringsHandler::transient_string_memory,&StringsHandler::string_memory);
//        Entity* entity = PushStruct(&EntitySytem::runtime_entities, Entity);
//        entity->ot.p = float3(0, 0, 0);
//        entity->ot.s = float3(1, 1, 1);
//        entity->ot.r = axis_angle(float3(0, 1, 0), 0);
//        ModelAsset *new_model_asset = PushStruct(&runtime_assets, ModelAsset);
//        entity->asset = new_model_asset;
  
        if(!ModelCache::DoesModelExist(path))
        {
            LoadedModelToModelAsset(loaded_model, new_model_asset);
            ModelCache::AddModel(path,&new_model_asset);
        }

        //TODO(Ray):Why does this fail sometimes!!
        UploadModelAssetToGPU(new_model_asset);
    }

    //TODO(Ray):Create a proper interface to FBXSDK
    ModelAsset* LoadModel(char* file_name,PlatformState* ps)
    {
        //1.  When we request to load a model first we go look for the definition file.
        //2. if one does not exist we create one on the fly
        //   NOTE(Ray):Def file has all the info and settings and also references other assets
        //     ie: textures to render that might.
        //     //WE always maintain a direct 1:1 relationship with the source asset for realtime
        //     reloading of asset.  If an asset is save in the DCC we should always see the update
        //     in the engine asap.  Without doing anything.  THe DCC is always the truth of the asset.
        //     The def file is a way for the game to render it properly with the after import settings.
        //     The DEF file ALWAYS references the source asset. ALWAYS.

        //LOAD MODEL from disk
        Yostr* base_path_to_data = BuildPathToAssets(&ps->string_state.string_memory, Directory_None);
        Yostr* mat_final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(file_name,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
        read_file_result mat_file_result =  PlatformReadEntireFile(mat_final_path);


    
    /*
        //Create a new material if we couldnt find the one at data
        if(mat_file_result.ContentSize <= 0)
        {
            //NOTE(Ray):If we dont have that file create it and fill out default materials than reload it.
            //Doint it this way just t oget things going will revist this later.
            Document d;
            // must pass an allocator when the object may need to allocate memory
            // go through mesh and create a json doc full of default materials for every mesh.
            d.SetObject();
            rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
            size_t sz = allocator.Size();

            d.AddMember("Mesh", "challenger.fbx", allocator);

            Value materials_json(kArrayType);
            
            Yostr* final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral("challenger.fbx",&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
            //After getting model get material definition from disk
            //Load model
            ModelAsset* new_model_asset;
            LoadedModel loaded_model = FBXSDKLoadModel(final_path->String, fbx_manager, fbx_scene, &ps->string_state.transient_string_memory,&ps->string_state.string_memory);
            LoadedModelToModelAsset(loaded_model, new_model_asset);
            for(int i = 0;i < new_model_asset->mesh_count;++i)
            {
                MeshAsset* ma = new_model_asset->meshes + i;
                Value obj(kObjectType);
                Value val(kObjectType);
                val.SetString(ma->name.String,(SizeType)ma->name.Length,allocator);
                obj.AddMember("name",val,allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value float_4_as_array(rapidjson::kArrayType);
                float_4_as_array.PushBack(0,allocator).PushBack(0,allocator).PushBack(0,allocator).PushBack(0,allocator);
                obj.AddMember("base_color",float_4_as_array,allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value texture_array(rapidjson::kArrayType);
                Value tex_val(kObjectType);
                tex_val.SetString("kart.png",(SizeType)8,allocator);
                texture_array.PushBack(tex_val,allocator);
                obj.AddMember("textures",texture_array,allocator);

                Value shader_obj(kObjectType);                
                rapidjson::Value object(rapidjson::kObjectType);
                shader_obj.AddMember("vs","diffuse_vs",allocator);
                shader_obj.AddMember("ps","diffuse_fs",allocator);

// create a rapidjson array type with similar syntax to std::vector
                rapidjson::Value slot_array(rapidjson::kArrayType);
                rapidjson::Value slot_object(rapidjson::kObjectType);
                slot_object.AddMember("id",0,allocator);
                slot_array.PushBack(slot_object,allocator);
                shader_obj.AddMember("slots",slot_array,allocator);

                obj.AddMember("shader",shader_obj,allocator);

                materials_json.PushBack(obj,allocator);
            }
            
            d.AddMember("Materials",materials_json , allocator);

            // Convert JSON document to string
            rapidjson::StringBuffer strbuf;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
            d.Accept(writer);
            const char* output = strbuf.GetString();
            int length = String_GetLength_Char((char*)output);
            PlatformFilePointer file{};
            PlatformWriteMemoryToFile(&file,"challenger.mat",(void*)output,length,true,"w");
//TODO(Ray):For now we just write a file to this location which is the default director on windows on osx
            ///Users/ray.garner/Library/Containers/dena.pedaltothemedal/Data
            //than we manually copy it over... obviously later we will need to fix that but need to continue focusing on the renderer for now.
            //come back to this lil quirk later.
            Assert(false);
            //PlatformOutput(true,"%s",output); 
        }
        
    }
        /*
        
// 1. Parse a JSON string into DOM.
        Document d;
        d.Parse((char*)mat_file_result.Content);
        if(!d.IsObject())
        {
            //Error handling
            assert(false);
        }

        const Value& mesh = d["Mesh"];
        Yostr* r = CreateStringFromLiteralConst(d["Mesh"].GetString(),&ps->string_state.transient_string_memory);
    
        Yostr* final_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(r->String,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
        //After getting model get material definition from disk

//Load model    
        LoadedModel loaded_model = FBXSDKLoadModel(final_path->String, fbx_manager, fbx_scene, &ps->string_state.transient_string_memory,&ps->string_state.string_memory);
        Entity* entity = PushStruct(&EntitySytem::runtime_entities, Entity);
        entity->ot.p = float3(0, 0, 0);
        entity->ot.s = float3(1, 1, 1);
        entity->ot.r = axis_angle(float3(0, 1, 0), 0);
        ModelAsset *new_model_asset = PushStruct(&runtime_assets, ModelAsset);
        entity->asset = new_model_asset;
        LoadedModelToModelAsset(loaded_model, new_model_asset);
        //TODO(Ray):Why does this fail sometimes!!
        UploadModelAssetToGPU(new_model_asset);
//END LOAD MODEL FROM DISK
        
//This next part gets and loads all the relevent information to render this object properly.
        //NOTE(Ray):
        //1. We dont have any good defaults or ways to generate this file from a newly imported mesh.
        //2. We will fail if the mesh is added to or removed need to have some mapping.
        //3. and add remove materials textures etc base on last use.
        //4. PSO's should also be allowed to be stored offline for immediate async creation upon startup
        //or at the users discretion.
        
        Texture uploaded_texture;
        RenderShader shader;
        const Value& materials = d["Materials"];
        int mesh_index = 0;
        for (auto& material : materials.GetArray())
        {
            if(mesh_index > new_model_asset->mesh_count - 1)Assert(false);//break;//TODO(Ray):materials need validation
//Load materials for each mesh index the materials should be in mesh index order for now
            MeshAsset* ma = new_model_asset->meshes + mesh_index;// YoyoGetVectorElement(MeshAsset,&,i);
            if(ma)
            {
                RenderMaterial mat_result = {};

                auto s = material["name"].GetString();
                
                mat_result.type = -1;//default opaque
                if(material.HasMember("type"))
                {
                    auto type = material["type"].GetInt();
                    mat_result.type = (int)type;
                }
                
                const Value& bc = material["base_color"];
                float4 base_color = float4(bc[0].GetFloat(),bc[1].GetFloat(),bc[2].GetFloat(),bc[3].GetFloat());
                mat_result.inputs.base_color = base_color;

                int j = 0;
                const Value& texture_array = material["textures"];
            
                for (auto& texture_value : texture_array.GetArray())
                {
                    Yostr* rrr = CreateStringFromLiteralConst(texture_value.GetString(),&ps->string_state.transient_string_memory);
                    Yostr* tex_path = AppendString(*base_path_to_data, *CreateStringFromLiteral(rrr->String,&ps->string_state.transient_string_memory), &ps->string_state.transient_string_memory);
                    if(!YoyoHashContains(&texture_hash,rrr->String,rrr->Length))
                    {
                        LoadedTexture loaded_tex;
                        GetImageFromDisk(tex_path->String,&loaded_tex);
                        uploaded_texture = UploadTextureToGPU(&loaded_tex);
                        YoyoPushBack(&loaded_textures,uploaded_texture);
                        Texture* tex_pointer = YoyoPeekVectorElement(Texture,&loaded_textures);
                        YoyoAddElementToHashTable(&texture_hash,rrr->String,rrr->Length,tex_pointer);
                    }
                    else
                    {
                        uploaded_texture = *YoyoGetElementByHash(Texture,&texture_hash,rrr->String,rrr->Length);
                    }
                    mat_result.texture_slots[mat_result.texture_count] = uploaded_texture;
                    mat_result.texture_count++;                   
                    j++;
                }

                const Value& value = material["shader"];
                const Value& vs_value = value["vs"];
                const Value& ps_value = value["ps"];
                const Value& slots = value["slots"];
            
                RenderShaderCode::InitShaderFromDefaultLib(&shader,(char*)vs_value.GetString(),(char*)ps_value.GetString());

                ShaderTextureSlot slot;
                slot.material_resource_id = 0;
                shader.texture_slot_count = 1;
                shader.texture_slots[0] = slot;
            
//Render pipeline descriptors init and setup based on material definitions
                RenderPipelineStateDesc render_pipeline_descriptor = RenderEncoderCode::CreatePipelineDescriptor(nullptr,nullptr,0);
                render_pipeline_descriptor.label = "test";
                render_pipeline_descriptor.vertex_function = shader.vs_object;
                render_pipeline_descriptor.fragment_function = shader.ps_object;
                render_pipeline_descriptor.depthAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
                render_pipeline_descriptor.stencilAttachmentPixelFormat = PixelFormatDepth32Float_Stencil8;
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
                
                //Vertex descriptions and application
                //TODO(Ray):So now we are going to return a vertex descriptor of our own and handle it
                //than set it back and let teh api do the translation for us. Thats the pattern here for pipeline
                //state creation... than we will do serialization to the pipeline desc for
                //pre creation of our pipeline states to save on load times and perfs if need be. but can save that
                //for much later stages.
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

//Create pipeline states    
                RenderPipelineState pipeline_state = RenderEncoderCode::NewRenderPipelineStateWithDescriptor(render_pipeline_descriptor);
                mat_result.pipeline_state = pipeline_state;

//create depth states            
                DepthStencilDescription depth_desc = RendererCode::CreateDepthStencilDescriptor();
                depth_desc.depthWriteEnabled = true;
                depth_desc.depthCompareFunction = compare_func_less;
                DepthStencilState depth_state = RendererCode::NewDepthStencilStateWithDescriptor(&depth_desc);
                mat_result.depth_stencil_state = depth_state;

                mat_result.shader = shader;
                ma->material = mat_result;
                mesh_index++;
            }
        }
        return new_model_asset;
    }
    */


    
};
