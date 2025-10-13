use winit::{
    application::ApplicationHandler,
    event_loop::{ActiveEventLoop, EventLoop},
    event::WindowEvent,
    window::{Window, WindowAttributes},
    dpi::PhysicalSize,
};

struct App { window: Option<Window> }

impl ApplicationHandler for App {
    fn resumed(&mut self, el: &ActiveEventLoop) {
        let attrs = WindowAttributes::default()
            .with_title("Step 1: plain window")
            .with_visible(true)
            .with_inner_size(PhysicalSize::new(640, 360));
        let window = el.create_window(attrs).expect("create_window");
        self.window = Some(window);
    }
    fn window_event(
        &mut self,
        el: &ActiveEventLoop,
        _id: winit::window::WindowId,
        event: WindowEvent,
    ) {
        if let WindowEvent::CloseRequested = event {
            el.exit();
        }
    }
}

fn main() {
    let event_loop = EventLoop::new().unwrap();
    let mut app = App { window: None };
    event_loop.run_app(&mut app).unwrap();
}
