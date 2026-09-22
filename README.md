# Vk-Raytracer
Vulkan raytracer

This is a personal project following the CPU based ray tracing listed out in Peter Shirley's books, as well as to learn Vulkans Graphics API.

![Image](public/test.png)

## Dependencies
- SDL3
- VulkanMemoryAllocator
- GLM
- Vulkan SDK
- CMake version 3.2+

## Todo
- GUI using IMGUI.
- Ability to load OBJ.
- Additional ray tracing features, e.g. better diffuse, reflections, glass etc.
- Benchmarking across multiple GPUs, using compute shaders and Vulkans ray tracing pipeline.
- Benchmarking across rendering techniques.

## Architecture
- Setting up the Vulkan pipeline was probably the most tedious and complicated part of this whole project, I made a great deal to separate each of it's setup components into separate objects so that they are easy to understand when used together but it is still not yet fully resolved. I've made diagrams to reason about the design of the setup and understand the relations between each component as I was developing.

![Diagram](public/diagram.jpg)

## Thoughts
- Currently, the program uses Vulkan's `compute shader` to calculate the paths and the pixels for the rays, I would like to eventually extend this to use `VK_KHR_ray_tracing_pipeline` extension.
- Currently, there is alot of screen tearing when moving the camera, the setup chooses FIFO mode when Vulkan is initialized which may be part of the problem, additionally, many image transitions are used in the rendering process which could also contribute to delayed frame swaps.
- The frame rate is absolutely atrocious to say the least, progressive accumulation of rays is probably done for a reason.
- The shader currently uses a brute force intersection loop for every object in the scene, every bounce of a ray check every sphere which is bad.

## Benchmarking
- Check back later.

## Resources
- Thanks to Sebastian Lague's series on Ray Tracing and Peter Shirley's Ray tracing trilogy. 

### Vulkan Specific
(Computer Graphics TU Wien Series)[https://www.youtube.com/watch?v=5VBVWCg7riQ]  
(constref Vulkan in 2 Hours)[https://www.youtube.com/watch?v=DC9FBRQKNck]  
(Introduction to Vulkan Compute Shaders)[https://www.youtube.com/watch?v=KN9nHo9kvZs]  
(Vulkan Tutorial)[https://vulkan-tutorial.com/]  
(Real Time RayTracing)[https://developer.nvidia.com/blog/vulkan-raytracing]  

