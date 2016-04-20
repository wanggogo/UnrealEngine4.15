// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	VulkanSwapChain.h: Vulkan viewport RHI definitions.
=============================================================================*/

#pragma once

class FVulkanQueue;

class FVulkanSwapChain
{
public:
	FVulkanSwapChain(VkInstance Instance, FVulkanDevice& InDevice, void* WindowHandle, EPixelFormat& InOutPixelFormat, uint32 Width, uint32 Height,
		uint32* InOutDesiredNumBackBuffers, TArray<VkImage>& OutImages);

	void Destroy();

	void Present(FVulkanQueue* Queue);

protected:
	VkSwapchainKHR SwapChain;
	FVulkanDevice& Device;

	PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
	PFN_vkQueuePresentKHR QueuePresentKHR;
	PFN_vkAcquireNextImageKHR AcquireNextImageKHR;

	VkSurfaceKHR Surface;

	int32 CurrentImageIndex;
	int32 SemaphoreIndex;
	TArray<FVulkanSemaphore*> ImageAcquiredSemaphore;
	//TArray<FVulkanSemaphore*> RenderingCompletedSemaphore;

	int32 AcquireImageIndex(FVulkanSemaphore** OutSemaphore);
	friend class FVulkanViewport;

	friend class FVulkanQueue;
};
