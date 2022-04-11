namespace Nodable
{
    /** Simple struct to initialise R */
    struct initializer
    {
        initializer();
        static void log_statistics();

    private:
        static bool s_initialized;
    };
}