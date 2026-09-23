# M22 plan 01 slice 08B3B: public stencil volume/composite state contract

Requires 08B3A.  Establish the public GPU-edge representation of the original
two-pass volume stencil fill and full-screen stencil composite, including
state validation, resource/failure rollback, recreation, and Recording
witnesses.  It cannot admit a shadow owner or read retail data.  It uses no
raw Direct3D or private Vulkan API, makes no visual/pixel claim, and leaves
projected/decal routes guarded.  A later 08B3C supplies the bounded original
owner and source ordering.
