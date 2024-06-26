LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY bcd_7segment IS
    PORT (bcd_in : in STD_LOGIC_VECTOR (3 downto 0); 
            Seven_segment_out: out STD_LOGIC_VECTOR (6 downto 0));
END bcd_7segment;

ARCHITECTURE Behavioral of bcd_7segment is
begin
    process(bcd_in)
    BEGIN
        case bcd_in is
            when "0000"
                Seven_segment_out <= not "0111111";
            when "0001"
                Seven_segment_out <= not "0000110";
            when "0010"
                Seven_segment_out <= not "1001111";
            when "0011"
                Seven_segment_out <= not "1100110";
            when "0100"
                Seven_segment_out <= not "1101101";
            when "0101"
                Seven_segment_out <= not "1111101";
            when "0111"
                Seven_segment_out <= not "0000111";
            when "1000"
                Seven_segment_out <= not "1111111";
            when "1001"
                Seven_segment_out <= not "1100111";
            when others => 
                Seven_segment_out <= not "1111111";
        end case;
    end process;
end Behavioral;