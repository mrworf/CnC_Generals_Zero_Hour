#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "original_gpu_edge.h"
#include "zh/renderer/recording_device.h"

#include <stdexcept>
#include <iostream>

namespace {
void check(bool value,const char* why) { if (!value) throw std::runtime_error(why); }

void vertex_growth(unsigned kind,zh::original_runtime::OriginalGpuEdge& edge)
{
    VertexBufferClass* old=nullptr;
    const unsigned short growth=static_cast<unsigned short>(
        DynamicVBAccessClass::Get_Default_Vertex_Count()+10);
    {
        DynamicVBAccessClass first(kind,dynamic_fvf_type,3);
        old=first.Peek_Buffer(); old->Add_Ref();
        {
            DynamicVBAccessClass::WriteLockClass lock(&first);
            lock.Get_Formatted_Vertex_Array()[0].x=42.0f;
        }
        bool busy=false;
        try { DynamicVBAccessClass illegal(kind,dynamic_fvf_type,4); }
        catch (const std::runtime_error&) { busy=true; }
        check(busy,"simultaneous original vertex access accepted");
        if (kind==BUFFER_TYPE_DYNAMIC_DX8) check(bool(edge.bind_vertex(old)),"edge did not retain old vertex bytes");
    }
    {
        DynamicVBAccessClass grown(kind,dynamic_fvf_type,growth);
        check(grown.Peek_Buffer()!=old && old->Num_Refs()>=1,
            "vertex pool reused a retained old generation");
        bool live_deinit=false;
        try { DynamicVBAccessClass::_Deinit(); }
        catch (const std::runtime_error&) { live_deinit=true; }
        check(live_deinit,"vertex pool deinitialized during a live access");
        VertexBufferClass::WriteLockClass lock(old,0);
        check(static_cast<VertexFormatXYZNDUV2*>(lock.Get_Vertex_Array())[0].x==42.0f,
            "retained original vertex contents changed during pool growth");
    }
    old->Release_Ref();
    if (kind==BUFFER_TYPE_DYNAMIC_DX8) edge.release_source_buffers();
    DynamicVBAccessClass::_Deinit();
}

void index_growth(unsigned kind,zh::original_runtime::OriginalGpuEdge& edge)
{
    IndexBufferClass* old=nullptr;
    const unsigned short growth=static_cast<unsigned short>(
        DynamicIBAccessClass::Get_Default_Index_Count()+10);
    {
        DynamicIBAccessClass first(kind,3);
        old=first.Peek_Buffer(); old->Add_Ref();
        {
            DynamicIBAccessClass::WriteLockClass lock(&first);
            lock.Get_Index_Array()[0]=42;
        }
        bool busy=false;
        try { DynamicIBAccessClass illegal(kind,4); }
        catch (const std::runtime_error&) { busy=true; }
        check(busy,"simultaneous original index access accepted");
        if (kind==BUFFER_TYPE_DYNAMIC_DX8) check(bool(edge.bind_index(old)),"edge did not retain old index bytes");
    }
    {
        DynamicIBAccessClass grown(kind,growth);
        check(grown.Peek_Buffer()!=old && old->Num_Refs()>=1,
            "index pool reused a retained old generation");
        bool live_deinit=false;
        try { DynamicIBAccessClass::_Deinit(); }
        catch (const std::runtime_error&) { live_deinit=true; }
        check(live_deinit,"index pool deinitialized during a live access");
        IndexBufferClass::WriteLockClass lock(old,0);
        check(lock.Get_Index_Array()[0]==42,"retained original index contents changed during pool growth");
    }
    old->Release_Ref();
    if (kind==BUFFER_TYPE_DYNAMIC_DX8) edge.release_source_buffers();
    DynamicIBAccessClass::_Deinit();
}
}

int main()
{
    try {
        zh::renderer::RecordingGpuDevice device;
        {
            zh::original_runtime::OriginalGpuEdge edge(device);
            for (unsigned kind:{BUFFER_TYPE_DYNAMIC_DX8,BUFFER_TYPE_DYNAMIC_SORTING}) {
                vertex_growth(kind,edge);
                index_growth(kind,edge);
            }
        }
        check(VertexBufferClass::Get_Total_Buffer_Count()==0 &&
            IndexBufferClass::Get_Total_Buffer_Count()==0 &&
            device.resource_counts().total()==0,"original dynamic growth retained resources");
        std::cout << "original dynamic growth ownership: ok\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
