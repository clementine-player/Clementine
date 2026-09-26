from clementine_remote.proto import pb


def test_new_enums_start_unspecified():
    for enum in (
        pb.ServerFeature,
        pb.RendererFeature,
        pb.OutputState,
        pb.StreamMode,
        pb.SeekMethod,
        pb.LoadStartState,
        pb.RendererState,
        pb.RendererErrorScope,
    ):
        assert enum.Name(0).endswith("_UNSPECIFIED")


def test_round_trip():
    msg = pb.Message(type=pb.RENDER_LOAD)
    msg.request_render_load.item.item_id = 7
    msg.request_render_load.start_state = pb.LOAD_START_STATE_PLAYING
    parsed = pb.Message.FromString(msg.SerializeToString())
    assert parsed.request_render_load.item.item_id == 7
