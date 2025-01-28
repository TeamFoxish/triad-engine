namespace Misc {
    funcdef void EventHandler(ref@ caller);

    class Event {
        private array<EventHandler@> callbacks;

        Event(const Event& other) delete;

        Event& opAssign(const Event&in) delete;

        void Subscribe(EventHandler@ callback) {
            callbacks.insertLast(callback);
        }

        void Unsubscribe(EventHandler@ callback) {
            const int idx = callbacks.findByRef(callback);
            if (idx < 0) {
                log_error("failed to unsubscribe from event. unable to find passed callback");
                return;
            }
            callbacks.removeAt(idx);
        }

        void Broadcast(ref@ caller) {
            for (int i = callbacks.length() - 1; i >= 0; --i) {
                callbacks[i](caller);
            }
        }

        void opCall(ref@ caller) {
            Broadcast(caller);
        }
    }
}
