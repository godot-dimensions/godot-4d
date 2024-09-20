# This file is for building as a Godot module.
def can_build(env, platform):
    # 4D depends on 3D to reuse things like Camera3D and AudioListener3D.
    return not env["disable_3d"]


def configure(env):
    pass


def get_doc_classes():
    return [
        # General.
        "Basis4D",
        "Camera4D",
        "Euler4D",
        "Geometry4D",
        "Node4D",
        "RenderingEngine4D",
        "RenderingServer4D",
        "Rotor4D",
        "Transform4D",
        "Vector4D",
        # Mesh.
        "ArrayTetraMesh4D",
        "ArrayWireMesh4D",
        "BoxTetraMesh4D",
        "BoxWireMesh4D",
        "OFFDocument",
        "OrthoplexTetraMesh4D",
        "OrthoplexWireMesh4D",
        "Material4D",
        "Mesh4D",
        "MeshInstance4D",
        "TetraMaterial4D",
        "TetraMesh4D",
        "WireMaterial4D",
        "WireMesh4D",
        # Physics.
        "BoxShape4D",
        "CapsuleShape4D",
        "CollisionShape4D",
        "ConcaveTetrameshShape4D",
        "ConvexHullShape4D",
        "CubinderShape4D",
        "CylinderShape4D",
        "DuocylinderShape4D",
        "OrthoplexShape4D",
        "Shape4D",
        "SphereShape4D",
    ]


def get_doc_path():
    return "addons/4d/doc_classes"


def get_icons_path():
    return "addons/4d/icons"
