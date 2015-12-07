import pi_rtvp.core
import pi_rtvp.input

def main():
    """Entry point for console-based camera control"""
    state = pi_rtvp.core.setup(18, 23)
    pi_rtvp.input.input_loop(state)
    pi_rtvp.core.cleanup(state)

if __name__ == "__main__":
    main()
