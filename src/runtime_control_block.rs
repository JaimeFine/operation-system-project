#[derive(Debug)]
enum State {
    Created,
    Ready,
    Running,
    Blocked,
    Finished,
}

#[derive(Debug)]
struct RCB {
    id: u32,
    name: String,
    time: u32,
    state: State,
}

impl RCB {
    // Creating a runtime:
    fn create_runtime(id: u32, name: &str) -> Self {
        Self {
            id,
            name: name.to_string(),
            time: 0,
            state: State::Created,
        }
    }

    // Execute a runtime:
    fn execute(&mut self, slice: u32) {
        self.time += slice;
        self.state = State::Running;
    }

    fn ready(&mut self) {
        self.state = State::Ready
    }

    // Block the runtime:
    fn block(&mut self) {
        self.state = State::Blocked;
    }

    // Finish the runtime:
    fn finish(&mut self) {
        self.state = State::Finished
    }
}