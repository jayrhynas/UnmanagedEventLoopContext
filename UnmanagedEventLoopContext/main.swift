import Foundation

class Person {
    var name: String
    
    init(name: String) {
        self.name = name
    }
}

let person = Person(name: "Jayson")

guard let loop = EventLoopCreate() else {
    fatalError("failed to create event loop")
}


EventLoopAddTask(loop, Unmanaged.passRetained(person).toOpaque(), { ptr in
    let person = Unmanaged<Person>.fromOpaque(ptr!).takeRetainedValue()
    print("ran task with \(person.name) via C callback")
})

loop.addTask {
    print("ran task with \(person.name) via Swift closure")
}

EventLoopCancel(loop)
EventLoopDestroy(loop)

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
