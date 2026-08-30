/**============================================================================
Name        : recording_event.hpp
Created on  : 19.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Event passed to the recording module.
============================================================================**/

/*
    RecordingEvent represents an event that should be persisted by RecordingModule.

    Recording is executed outside the processing modules. Market-data and execution
    modules publish events into a recording queue, while RecordingModule is the only
    component responsible for invoking the recorder.

    Data Flow:

        BookBuilderModule
               |
               | MarketEvent
               v
        recordingEventQueue
               |
               v
        RecordingModule
               |
               v
            Recorder


        ExecutionModule
               |
               | ExecutionReport
               v
        recordingEventQueue
               |
               v
        RecordingModule
               |
               v
            Recorder

    Responsibilities:

        - provide a common queue item for events that must be recorded;
        - preserve the original domain event;
        - decouple event producers from the recorder implementation.

    RecordingEvent is a transport object. It does not perform recording itself.
*/

#ifndef FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP
#define FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP

#include "execution_report.hpp"
#include "order.hpp"
#include "model/market_event.hpp"

#include <variant>

namespace trading::recording
{
    using RecordingEvent = std::variant<
        market_data::MarketEvent,
        execution::OrderRequest,
        execution::ExecutionReport
    >;
}

#endif //FINANCETECHNOLOGYPROJECTS_RECORDING_EVENT_HPP