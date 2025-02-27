/**
 * This file is part of Special K.
 *
 * Special K is free software : you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by The Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Special K is distributed in the hope that it will be useful,
 *
 * But WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Special K.
 *
 *   If not, see <http://www.gnu.org/licenses/>.
 *
**/
#pragma once

#include <SpecialK/render/d3d12/d3d12_interfaces.h>

using
D3D12Device_CreateGraphicsPipelineState_pfn = HRESULT
(STDMETHODCALLTYPE *)(ID3D12Device*,
                const D3D12_GRAPHICS_PIPELINE_STATE_DESC*,
                      REFIID,void**);

using
D3D12Device2_CreatePipelineState_pfn = HRESULT
(STDMETHODCALLTYPE *)(ID3D12Device2*,
                 const D3D12_PIPELINE_STATE_STREAM_DESC*,
                       REFIID,void**);

using
D3D12Device_CreateShaderResourceView_pfn = void
(STDMETHODCALLTYPE *)(ID3D12Device*,ID3D12Resource*,
                const D3D12_SHADER_RESOURCE_VIEW_DESC*,
                      D3D12_CPU_DESCRIPTOR_HANDLE);

using
D3D12Device_CreateUnorderedAccessView_pfn = void
(STDMETHODCALLTYPE *)(ID3D12Device*,ID3D12Resource*,
                      ID3D12Resource*,
                const D3D12_UNORDERED_ACCESS_VIEW_DESC*,
                      D3D12_CPU_DESCRIPTOR_HANDLE);

using
D3D12Device_CreateRenderTargetView_pfn = void
(STDMETHODCALLTYPE *)(ID3D12Device*,ID3D12Resource*,
                const D3D12_RENDER_TARGET_VIEW_DESC*,
                      D3D12_CPU_DESCRIPTOR_HANDLE);

using
D3D12Device_GetResourceAllocationInfo_pfn = D3D12_RESOURCE_ALLOCATION_INFO
(STDMETHODCALLTYPE *)(ID3D12Device*,UINT,UINT,const D3D12_RESOURCE_DESC*);

using
D3D12Device_CreateCommittedResource_pfn = HRESULT
(STDMETHODCALLTYPE *)(ID3D12Device*,const D3D12_HEAP_PROPERTIES*,D3D12_HEAP_FLAGS,
                                    const D3D12_RESOURCE_DESC*,D3D12_RESOURCE_STATES,
                                    const D3D12_CLEAR_VALUE*,REFIID,void**);

using
D3D12Device_CreatePlacedResource_pfn = HRESULT
(STDMETHODCALLTYPE *)(ID3D12Device*,ID3D12Heap*,
                       UINT64,const D3D12_RESOURCE_DESC*,
                                    D3D12_RESOURCE_STATES,
                              const D3D12_CLEAR_VALUE*,REFIID,void**);

using
D3D12Device_CreateCommandAllocator_pfn = HRESULT
(STDMETHODCALLTYPE *)(ID3D12Device*,D3D12_COMMAND_LIST_TYPE,
                      REFIID,void**);


extern D3D12Device_CreateGraphicsPipelineState_pfn
       D3D12Device_CreateGraphicsPipelineState_Original;
extern D3D12Device2_CreatePipelineState_pfn
       D3D12Device2_CreatePipelineState_Original;
extern D3D12Device_CreateShaderResourceView_pfn
       D3D12Device_CreateShaderResourceView_Original;
extern D3D12Device_CreateRenderTargetView_pfn
       D3D12Device_CreateRenderTargetView_Original;
extern D3D12Device_GetResourceAllocationInfo_pfn
       D3D12Device_GetResourceAllocationInfo_Original;
extern D3D12Device_CreateCommittedResource_pfn
       D3D12Device_CreateCommittedResource_Original;
extern D3D12Device_CreatePlacedResource_pfn
       D3D12Device_CreatePlacedResource_Original;
extern D3D12Device_CreateCommandAllocator_pfn
       D3D12Device_CreateCommandAllocator_Original;

bool SK_D3D12_HookDeviceCreation (void);
void SK_D3D12_InstallDeviceHooks (ID3D12Device* pDev12);

static inline constexpr GUID SKID_D3D12IgnoredTextureCopy = { 0x3d5298cb, 0xd8f0,  0x7233, { 0xa1, 0x9d, 0xb1, 0xd5, 0x97, 0x92, 0x00, 0x70 } };

void SK_D3D12_CommitUploadQueue (ID3D12GraphicsCommandList *pCmdList);