LIBRARY ieee;
USE ieee.std_logic_1164.all;

ENTITY LEDR IS
	PORT (gpioinput : in STD_LOGIC_VECTOR (3 downto 0); 
			ledoutput: out STD_LOGIC_VECTOR (9 downto 0));
END LEDR;

ARCHITECTURE Behavioral of LEDR is
begin
	process(input)
	BEGIN
		case input is
			when "0000" => output <= "0000000000";
			when "0001" => output <= "0000000001";
			when "0010" => output <= "0000000011";
			when "0011" => output <= "0000000111";
			when "0100" => output <= "0000001111";
			when "0101" => output <= "0000011111";
			when "0110" => output <= "0000111111";
			when "0111" => output <= "0001111111";
			when "1000" => output <= "0011111111";
			when "1001" => output <= "0111111111";
			when others => output <= "1111111111";
		end case;
	end process;
end Behavioral;