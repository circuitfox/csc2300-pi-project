from hypothesis import given
import hypothesis.strategies as st
import pi_rtvp.motor as motor
import math

@given(st.integers())
def test_angle_to_dutycycle_range(angle):
    """Test that angle_to_dutycycle returns results in the proper range"""
    assert(0 <= motor.angle_to_dutycycle(angle) <= 100)

@given(st.integers())
def test_dutycycle_to_angle_range(dc):
    """Test that dutycycle_to_angle returns angles not less than -180."""
    assert(-180 <= motor.dutycycle_to_angle(dc))

@given(st.integers(-180, 3420))
def test_angle_dutycycle_inverse(angle):
    """Test the results of angle_to_dutycycle against its inverse, dutycycle_to_angle,
    angle = ((dutycycle / 5) - 1) * 180. Passing of this test implies that
    dutycycle_to_angle gives the proper results given angle_to_dutycycle.
    """
    assert(math.isclose(motor.dutycycle_to_angle(motor.angle_to_dutycycle(angle)), angle))
