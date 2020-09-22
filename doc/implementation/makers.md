# About sill's meshes

The `MeshComponent` provides a generic interface to set meshes.
This allows the user to load any custom data into the engine.

However, for general usage (or prototyping), the engine provides
a way to make spheres, cubes or load GLB2 files.
The concept to do so is called `makers`.

```C++
auto& entity = engine.make<sill::Entity>();
auto& meshComponent = entity.make<sill::MeshComponent>();
sill::makers::glbMeshMaker("model.glb")(meshComponent);
```

All the makers create a intermediate function:

```C++
auto makeFunction = sill::makers::sphereMeshMaker(32u, 1.f);
```

This function has pre-baked results and expects a `IMesh`
as an argument to apply its data. If you intend to create
the same mesh multiple times (without instanciating),
you should save that function somewhere.
