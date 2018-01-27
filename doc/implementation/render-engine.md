# magma::RenderEngine

__NOTE__: The following information might be wrong.

The render engine uses a light linked list (LLL), as described by
[Insomniac Games](http://www.insomniacgames.com/siggraph-real-time-lighting-via-light-linked-list/).

This technique enables a deferred renderer engine to render custom
materials (such as translucent ones) over complex lighting. The main
idea is to provide for each fragment a list of lights that concerns it.
The basic deferred lighting is itself optimized by this method, each
fragment having to compute just the relevent lights. As a last pass,
custom materials can use the same list to be rendered.

We are using a right-handed system a positive Z axis being up.

## Stages

- **G-Buffer** constructs every buffer needed to render the lights over the fully opaque meshes ; 
- **Epiphany** mixes the G-Buffer with the different lights to create the final image ;
- **Present** displays the final image.
