def can_build(env, platform):
    # 4D depends on 3D to reuse things like Camera3D and AudioListener3D.
    return not env["disable_3d"]


def configure(env):
    pass


def get_doc_classes():
    return []


def get_doc_path():
    return "doc_classes"


def get_icons_path():
    return "addons/4d/icons"
