# magma::RenderEngine

The render engine uses a light linked list (LLL), as described by
[Insomniac Games](http://www.insomniacgames.com/siggraph-real-time-lighting-via-light-linked-list/).

This technique enables a deferred renderer engine to render custom
materials (such as translucent ones) over complex lighting. The main
idea is to provide for each fragment a list of lights that concerns it.
The basic deferred lighting is itself optimized by this method, each
fragment having to compute just the relevent lights. As a last pass,
custom materials can use the same list to be rendered.    
