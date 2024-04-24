module LED_GPIO(
    input [3:0] gpioinput,
    output reg [9:0] ledoutput
);

// Combinational logic block for updating the output
always @* begin
    ledoutput = decode_gpio_to_led(gpioinput);
end

// Function to convert GPIO input to LED output pattern
function [9:0] decode_gpio_to_led(input [3:0] gpio);
    begin
        case (gpio)
            4'b0000: decode_gpio_to_led = 10'b0000000000;
            4'b0001: decode_gpio_to_led = 10'b0000000001;
            4'b0010: decode_gpio_to_led = 10'b0000000011;
            4'b0011: decode_gpio_to_led = 10'b0000000111;
            4'b0100: decode_gpio_to_led = 10'b0000001111;
            4'b0101: decode_gpio_to_led = 10'b0000011111;
            4'b0110: decode_gpio_to_led = 10'b0000111111;
            4'b0111: decode_gpio_to_led = 10'b0001111111;
            4'b1000: decode_gpio_to_led = 10'b0011111111;
            4'b1001: decode_gpio_to_led = 10'b0111111111;
            default: decode_gpio_to_led = 10'b1111111111; // All LEDs on for undefined cases
        endcase
    end
endfunction

endmodule
