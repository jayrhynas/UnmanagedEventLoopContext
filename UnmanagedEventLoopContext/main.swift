import Foundation

guard let eventLoop = EventLoopCreate() else {
    fatalError("failed to create event loop")
}

class Person {
    var name: String
    
    init(name: String) {
        self.name = name
    }
}

let person = Person(name: "Jayson")

EventLoopAddTask(eventLoop, Unmanaged.passRetained(person).toOpaque(), { ptr in
    let person = Unmanaged<Person>.fromOpaque(ptr!).takeRetainedValue()
    print("ran task with \(person.name) via C callback")
})

eventLoop.addTask {
    print("ran task with \(person.name) via Swift closure")
}

EventLoopCancel(eventLoop)
EventLoopDestroy(eventLoop)

extension EventLoop {
    func addTask(task: @escaping () -> Void) {
        class Context {
            let task: () -> Void
            
            init(task: @escaping () -> Void) {
                self.task = task
            }
        }
        
        let context = Context(task: task)
        
        EventLoopAddTask(self, Unmanaged.passRetained(context).toOpaque()) { ptr in
            let local = Unmanaged<Context>.fromOpaque(ptr!).takeRetainedValue()
            local.task()
        }
    }
}
