def can_build(env, platform):
    # 4D depends on 3D to reuse things like Camera3D and AudioListener3D.
    return not env["disable_3d"]


def configure(env):
    pass


def get_doc_classes():
    return [
        "Basis4D",
        "BoxShape4D",
        "CapsuleShape4D",
        "CollisionShape4D",
        "CubinderShape4D",
        "CylinderShape4D",
        "DuocylinderShape4D",
        "Euler4D",
        "Node4D",
        "Shape4D",
        "SphereShape4D",
        "Transform4D",
        "Vector4D",
    ]


def get_doc_path():
    return "doc_classes"


def get_icons_path():
    return "addons/4d/icons"
