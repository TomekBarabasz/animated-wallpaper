use std::sync::Arc;
use wgpu::{
    Surface, SurfaceConfiguration, Device, Queue,
    //Adapter, Device, Instance, InstanceDescriptor, MemoryHints, Queue, Surface,
    //SurfaceConfiguration, SurfaceTargetUnsafe,
};
use winit::{
    window::Window
};

pub struct WGpuState {
    /*
    surface: wgpu::Surface<'static>,
    device: wgpu::Device,
    queue: wgpu::Queue,
    config: wgpu::SurfaceConfiguration,
    */
    // instance: Instance,
    // adapter: Adapter,
    window: Arc<Window>,
}

impl WGpuState {
    pub async fn new(window: Arc<Window>) -> anyhow::Result<Self> {
        Ok(Self {
            window,
        })
    }
    pub fn resize(&mut self, _width: u32, _height: u32) {
        // todo
    }
    pub fn render(&mut self) {
        self.window.request_redraw();
    }
}
