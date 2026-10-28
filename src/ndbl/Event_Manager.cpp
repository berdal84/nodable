#include "Event_Manager.h"

#include "Asserts.h"
#include "Task_Manager.h"

#define VERIFY_EVENT_MANAGER_IS_INITIALIZED() VERIFY( g_event_manager != nullptr, "g_event_manager is not initialized, did you cann event_manager_init() ?")

namespace ndbl
{

static Event_Manager* g_event_manager = nullptr;

Event_Manager* event_manager_init()
{
    VERIFY(g_event_manager == nullptr, "Cannot be called twice"); // singleton

    g_event_manager = bdc::memory_new<Event_Manager>();
    array_init(g_event_manager->event_queue_ring, 128);
    g_event_manager->event_queue_front = 0;
    g_event_manager->event_queue_back  = 0;

    return g_event_manager;
}

Event_Manager* event_manager()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    return g_event_manager;
}

void  event_manager_shutdown()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    array_release(g_event_manager->event_queue_ring);
    bdc::memory_delete(g_event_manager);
    g_event_manager = nullptr;
}

void event_manager_push_event(Event event)
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    u32_t next_pos = (g_event_manager->event_queue_back + 1) % g_event_manager->event_queue_ring.capacity;
    assert( next_pos != g_event_manager->event_queue_front); // Queue is full, increase event_queue_ring size

    // Note: in case ring is empty, we move the front/back cursors to the begining
    //       that way we avoid a possible cache miss if events are stored at the very end and at the very beginning of the memory.
    //
    //       ----------------------------||-- (only few slots available at the end)
    //
    //                becomes vvvv
    //       ||------------------------------ (100% avaliable)
    //

    if( g_event_manager->event_queue_front == g_event_manager->event_queue_back )
    {
        g_event_manager->event_queue_front = 0;
        next_pos = 0;
    }

    g_event_manager->event_queue_ring[next_pos] = event;
    g_event_manager->event_queue_back           = next_pos;
}

const Event& event_manager_pop_event()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    if ( g_event_manager->event_queue_front == g_event_manager->event_queue_back )
    {
        return null_event;
    }

    const Event& next_event = g_event_manager->event_queue_ring[g_event_manager->event_queue_front];
    g_event_manager->event_queue_front = (g_event_manager->event_queue_front + 1) % g_event_manager->event_queue_ring.capacity;

    return next_event;
}

void event_manager_push_delayed_event( Event event, u64_t delay_in_ms)
{
    auto task = [event]() -> void {
        event_manager_push_event(event);
    };
    task_manager_schedule_task(task, delay_in_ms);
}

} // namespace ndbl