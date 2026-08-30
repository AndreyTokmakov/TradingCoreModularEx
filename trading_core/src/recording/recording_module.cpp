/**============================================================================
Name        : recording_module.cpp
Created on  : 25.08.2026
Author      : Andrei Tokmakov
Version     : 1.0
Copyright   : Your copyright notice
Description : Records market events on the recording thread.
============================================================================**/

#include "recording_module.hpp"

namespace trading::recording
{
    RecordingModule::RecordingModule(IRecorder& recorder,
                                     concurrency::Queue<market_data::MarketEvent>& queue):
        recorder { recorder },
        queue { queue }
    {
    }

    void RecordingModule::run() const
    {
        market_data::MarketEvent event;
        while (queue.waitPop(event)) {
            recorder.record(event);
        }
    }
}