main()
{
   static char str[2] = "X";

   write(1, "Starting\n", 9);

   str[0] = 'A' +(fork()!=0);

   for(;;)
      write(1, str, 1);
}
